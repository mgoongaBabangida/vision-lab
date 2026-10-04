#include "visionlab/opencv/feature_matching.hpp"
#include <opencv2/calib3d.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace visionlab
{
namespace
{
constexpr int maximum_dimension = 640;
constexpr std::size_t displayed_matches = 60;
constexpr double rotation_degrees = 25.0;
constexpr double scale_factor = 0.80;
constexpr float ratio_threshold = 0.75f;

struct MatchingData
{
    cv::Mat source, target, transform;
    cv::Mat source_descriptors, target_descriptors;
    std::vector<cv::KeyPoint> source_points, target_points;
    std::vector<cv::DMatch> matches, inliers;
    std::shared_ptr<FeatureMatchReport> report;
    FeatureMethod method;
};

class MatchingStage final : public Stage
{
public:
    MatchingStage(std::shared_ptr<MatchingData> data, int step) : data_(std::move(data)), step_(step)
    {
    }
    std::string_view name() const noexcept override
    {
        switch (step_)
        {
        case 0:
            return "Image pair: original / rotated and scaled";
        case 1:
            return "Keypoints and descriptors";
        case 2:
            return "Descriptor matches: ratio < 0.75";
        default:
            return "RANSAC homography inliers";
        }
    }
    void process(FrameResult& result) override
    {
        if (step_ == 0)
        {
            *data_->report = {};
            data_->source_points.clear();
            data_->target_points.clear();
            data_->matches.clear();
            data_->inliers.clear();
            data_->source_descriptors.release();
            data_->target_descriptors.release();
            Image& image = result.frame.image;
            const cv::Mat view(image.height(), image.width(), CV_8UC3, image.data(), image.stride_bytes());
            const double factor = std::min(1.0, static_cast<double>(maximum_dimension) / std::max(view.cols, view.rows));
            cv::resize(view, data_->source, cv::Size(std::max(1, cvRound(view.cols * factor)), std::max(1, cvRound(view.rows * factor))), 0,
                       0, cv::INTER_AREA);
            const cv::Point2f center((data_->source.cols - 1) * 0.5f, (data_->source.rows - 1) * 0.5f);
            data_->transform = cv::getRotationMatrix2D(center, rotation_degrees, scale_factor);
            cv::warpAffine(data_->source, data_->target, data_->transform, data_->source.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT,
                           cv::Scalar(25, 25, 25));
        }
        else if (step_ == 1)
        {
            cv::Ptr<cv::Feature2D> detector;
            if (data_->method == FeatureMethod::Sift)
            {
                detector = cv::SIFT::create(600);
            }
            else
            {
                detector = cv::ORB::create(600);
            }
            cv::Mat source_gray, target_gray;
            cv::cvtColor(data_->source, source_gray, cv::COLOR_BGR2GRAY);
            cv::cvtColor(data_->target, target_gray, cv::COLOR_BGR2GRAY);
            // Avoid ORB's pyramid limitations on tiny inputs; their report is simply empty.
            if (std::min(source_gray.rows, source_gray.cols) >= 32)
            {
                detector->detectAndCompute(source_gray, cv::noArray(), data_->source_points, data_->source_descriptors);
                detector->detectAndCompute(target_gray, cv::noArray(), data_->target_points, data_->target_descriptors);
            }
            data_->report->source_keypoints = data_->source_points.size();
            data_->report->target_keypoints = data_->target_points.size();
        }
        else if (step_ == 2)
        {
            data_->matches.clear();
            if (!data_->source_descriptors.empty() && data_->target_descriptors.rows >= 2)
            {
                cv::BFMatcher matcher(data_->method == FeatureMethod::Sift ? cv::NORM_L2 : cv::NORM_HAMMING);
                std::vector<std::vector<cv::DMatch>> neighbors;
                matcher.knnMatch(data_->source_descriptors, data_->target_descriptors, neighbors, 2);
                for (const std::vector<cv::DMatch>& pair : neighbors)
                {
                    if (pair.size() == 2 && pair[0].distance < ratio_threshold * pair[1].distance)
                    {
                        data_->matches.push_back(pair[0]);
                    }
                }
                std::sort(data_->matches.begin(), data_->matches.end(),
                          [](const cv::DMatch& a, const cv::DMatch& b)
                          {
                              return a.distance != b.distance ? a.distance < b.distance : a.queryIdx < b.queryIdx;
                          });
            }
            data_->report->ratio_matches = data_->matches.size();
        }
        else if (step_ == 3)
        {
            data_->inliers.clear();
            if (data_->matches.size() >= 4)
            {
                std::vector<cv::Point2f> source, target;
                for (const cv::DMatch& match : data_->matches)
                {
                    source.push_back(data_->source_points[match.queryIdx].pt);
                    target.push_back(data_->target_points[match.trainIdx].pt);
                }
                cv::Mat mask;
                const cv::Mat homography = cv::findHomography(source, target, cv::RANSAC, 3.0, mask);
                if (!homography.empty())
                {
                    for (std::size_t index = 0; index < data_->matches.size(); ++index)
                    {
                        if (mask.at<unsigned char>(static_cast<int>(index)))
                        {
                            data_->inliers.push_back(data_->matches[index]);
                        }
                    }
                }
            }
            data_->report->inliers = data_->inliers.size();
            std::vector<double> errors;
            for (const cv::DMatch& match : data_->inliers)
            {
                const cv::Point2f a = data_->source_points[match.queryIdx].pt;
                const cv::Point2f b = data_->target_points[match.trainIdx].pt;
                const cv::Mat& t = data_->transform;
                const double x = t.at<double>(0, 0) * a.x + t.at<double>(0, 1) * a.y + t.at<double>(0, 2);
                const double y = t.at<double>(1, 0) * a.x + t.at<double>(1, 1) * a.y + t.at<double>(1, 2);
                errors.push_back(std::hypot(x - b.x, y - b.y));
            }
            if (!errors.empty())
            {
                std::sort(errors.begin(), errors.end());
                data_->report->median_known_transform_error = (errors[(errors.size() - 1) / 2] + errors[errors.size() / 2]) * 0.5;
            }
        }
        render(result);
    }

private:
    void render(FrameResult& result)
    {
        cv::Mat left, right;
        if (step_ == 1)
        {
            cv::drawKeypoints(data_->source, data_->source_points, left, cv::Scalar(0, 255, 255),
                              cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
            cv::drawKeypoints(data_->target, data_->target_points, right, cv::Scalar(0, 255, 255),
                              cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        }
        else
        {
            left = data_->source;
            right = data_->target;
        }
        cv::Mat pair;
        std::size_t shown = 0;
        if (step_ >= 2)
        {
            const std::vector<cv::DMatch>& all = step_ == 2 ? data_->matches : data_->inliers;
            shown = std::min(all.size(), displayed_matches);
            const std::vector<cv::DMatch> subset(all.begin(), all.begin() + shown);
            cv::drawMatches(data_->source, data_->source_points, data_->target, data_->target_points, subset, pair,
                            cv::Scalar(80, 230, 100), cv::Scalar::all(0), std::vector<char>(),
                            cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
        }
        else
        {
            cv::hconcat(left, right, pair);
        }
        cv::Mat display;
        cv::copyMakeBorder(pair, display, 54, 0, 0, 0, cv::BORDER_CONSTANT, cv::Scalar(15, 15, 15));
        std::ostringstream text;
        text.imbue(std::locale::classic());
        text << (data_->method == FeatureMethod::Sift ? "SIFT" : "ORB") << " kp=" << data_->report->source_keypoints << "/"
             << data_->report->target_keypoints << " ratio=" << data_->report->ratio_matches << " inliers=" << data_->report->inliers;
        cv::putText(display, text.str(), cv::Point(8, 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(230, 230, 230), 1, cv::LINE_AA);
        std::ostringstream status;
        status.imbue(std::locale::classic());
        status << "left: original | right: " << rotation_degrees << " deg, x" << scale_factor;
        if (step_ >= 2)
        {
            status << " | shown=" << shown;
        }
        if (step_ == 3)
        {
            if (data_->report->median_known_transform_error)
            {
                status << " | true error=" << std::fixed << std::setprecision(2) << *data_->report->median_known_transform_error << "px";
            }
            else
            {
                status << " | no valid model";
            }
        }
        cv::putText(display, status.str(), cv::Point(8, 42), cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(230, 230, 230), 1, cv::LINE_AA);
        Image output(display.cols, display.rows);
        cv::Mat view(output.height(), output.width(), CV_8UC3, output.data(), output.stride_bytes());
        display.copyTo(view);
        result.frame.image = std::move(output);
        result.boxes.clear();
    }
    std::shared_ptr<MatchingData> data_;
    int step_;
};
} // namespace

void add_feature_matching(Pipeline& pipeline, FeatureMethod method, std::shared_ptr<FeatureMatchReport> report)
{
    if (method != FeatureMethod::Sift && method != FeatureMethod::Orb)
    {
        throw std::invalid_argument("Unsupported feature method");
    }
    const auto data = std::make_shared<MatchingData>();
    data->method = method;
    data->report = report ? std::move(report) : std::make_shared<FeatureMatchReport>();
    for (int step = 0; step < 4; ++step)
    {
        pipeline.add(std::make_unique<MatchingStage>(data, step));
    }
}

} // namespace visionlab

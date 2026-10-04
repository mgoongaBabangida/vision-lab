#include "visionlab/opencv/feature_matching.hpp"
#include <opencv2/imgproc.hpp>
#include "visionlab/opencv/video_source.hpp"
#include "visionlab/opencv/moments_stage.hpp"
#include "visionlab/opencv/rotated_rectangle_stage.hpp"
#include "visionlab/opencv/contour_hierarchy_stage.hpp"
#include "visionlab/opencv/contours_stage.hpp"
#include "visionlab/opencv/connected_components_stage.hpp"
#include "visionlab/opencv/gradient_direction_stage.hpp"
#include "visionlab/opencv/image_source.hpp"
#include "visionlab/opencv/mean_blur_stage.hpp"
#include "visionlab/opencv/binary_threshold_stage.hpp"
#include "visionlab/opencv/hsv_color_stage.hpp"
#include "visionlab/opencv/adaptive_threshold_stage.hpp"
#include "visionlab/opencv/otsu_threshold_stage.hpp"
#include "visionlab/opencv/erosion_stage.hpp"
#include "visionlab/opencv/dilation_stage.hpp"
#include "viewer_session.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <functional>
#include <iostream>
#include <stdexcept>

namespace
{
void require(bool value, const char* message)
{
    if (!value)
    {
        throw std::runtime_error(message);
    }
}

void test_grayscale()
{
    const std::array<std::array<std::uint8_t, 3>, 5> colors{{{255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {255, 255, 255}, {0, 0, 0}}};
    const std::array<std::uint8_t, 5> expected{29, 150, 76, 255, 0};
    // An odd width and reversed second row also exercise dimensions and row stride.
    visionlab::Frame frame{17, 0.5, visionlab::Image(5, 2)};
    for (std::size_t row = 0; row < 2; ++row)
    {
        for (std::size_t column = 0; column < colors.size(); ++column)
        {
            const std::size_t color = row == 0 ? column : colors.size() - 1 - column;
            std::copy(colors[color].begin(), colors[color].end(), frame.image.data() + row * frame.image.stride_bytes() + column * 3);
        }
    }
    const visionlab::app::PipelineCatalog catalog;
    visionlab::Pipeline pipeline = catalog.create("practice-01-grayscale");
    const visionlab::FrameResult result = pipeline.process(frame, true);
    require(result.frame.index == 17 && result.frame.timestamp_seconds == frame.timestamp_seconds && result.frame.image.width() == 5 &&
                result.frame.image.height() == 2,
            "Grayscale preserves frame identity, time, and dimensions");
    require(result.snapshots.size() == 2 && result.snapshots[1].name == "Grayscale", "Grayscale has an inspectable stage snapshot");
    require(std::equal(frame.image.data(), frame.image.data() + frame.image.size_bytes(), result.snapshots[0].image.data()),
            "Source snapshot preserves original colors");
    for (std::size_t row = 0; row < 2; ++row)
    {
        for (std::size_t column = 0; column < colors.size(); ++column)
        {
            const std::size_t color = row == 0 ? column : colors.size() - 1 - column;
            const std::size_t offset = row * frame.image.stride_bytes() + column * 3;
            for (std::size_t channel = 0; channel < 3; ++channel)
            {
                require(result.frame.image.data()[offset + channel] == expected[color] &&
                            result.snapshots[1].image.data()[offset + channel] == expected[color],
                        "Known BGR colors become the expected three equal grayscale channels");
            }
        }
    }
}
void test_mean_blur()
{
    visionlab::Frame frame{23, 0.75, visionlab::Image(7, 5)};
    for (std::size_t offset = 0; offset < frame.image.size_bytes(); offset += 3)
    {
        frame.image.data()[offset] = 10;
        frame.image.data()[offset + 1] = 20;
        frame.image.data()[offset + 2] = 30;
    }
    const std::size_t center = 2 * frame.image.stride_bytes() + 3 * 3;
    for (std::size_t channel = 0; channel < 3; ++channel)
    {
        frame.image.data()[center + channel] += 90;
    }
    visionlab::Pipeline pipeline;
    pipeline.add(std::make_unique<visionlab::MeanBlurStage>());
    const visionlab::FrameResult result = pipeline.process(frame, true);
    require(result.frame.index == frame.index && result.frame.timestamp_seconds == frame.timestamp_seconds &&
                result.frame.image.width() == 7 && result.frame.image.height() == 5,
            "Mean blur preserves frame metadata and dimensions");
    require(result.snapshots.size() == 2 &&
                std::equal(frame.image.data(), frame.image.data() + frame.image.size_bytes(), result.snapshots[0].image.data()),
            "Mean blur preserves the original source snapshot");
    for (std::size_t row = 0; row < 5; ++row)
    {
        for (std::size_t column = 0; column < 7; ++column)
        {
            // Only neighborhoods containing the bright center receive its extra 90 / 9 = 10.
            const bool contains_center = row >= 1 && row <= 3 && column >= 2 && column <= 4;
            const std::size_t offset = row * frame.image.stride_bytes() + column * 3;
            for (std::size_t channel = 0; channel < 3; ++channel)
            {
                const std::size_t expected = 10 * (channel + 1) + (contains_center ? 10 : 0);
                require(result.frame.image.data()[offset + channel] == expected &&
                            result.snapshots[1].image.data()[offset + channel] == expected,
                        "3x3 mean blur averages each channel and copies the pixels back for display");
            }
        }
    }
}
void test_binary_threshold()
{
    const std::array<std::uint8_t, 5> input{0, 126, 127, 128, 255};
    const std::array<std::uint8_t, 5> expected{0, 0, 0, 255, 255};
    visionlab::Frame frame{31, 1.25, visionlab::Image(5, 1)};
    for (std::size_t pixel = 0; pixel < input.size(); ++pixel)
    {
        std::fill_n(frame.image.data() + pixel * 3, 3, input[pixel]);
    }
    visionlab::Pipeline pipeline;
    pipeline.add(std::make_unique<visionlab::BinaryThresholdStage>());
    const visionlab::FrameResult result = pipeline.process(frame, true);
    require(result.frame.index == frame.index && result.frame.timestamp_seconds == frame.timestamp_seconds &&
                result.frame.image.width() == 5 && result.frame.image.height() == 1,
            "Binary threshold preserves frame metadata and dimensions");
    require(result.snapshots.size() == 2 &&
                std::equal(frame.image.data(), frame.image.data() + frame.image.size_bytes(), result.snapshots[0].image.data()),
            "Binary threshold preserves the source snapshot");
    for (std::size_t pixel = 0; pixel < input.size(); ++pixel)
    {
        for (std::size_t channel = 0; channel < 3; ++channel)
        {
            require(result.frame.image.data()[pixel * 3 + channel] == expected[pixel] &&
                        result.snapshots[1].image.data()[pixel * 3 + channel] == expected[pixel],
                    "Threshold cutoff is strict and the mask is written to all display channels");
        }
    }
}
void test_morphology()
{
    visionlab::Frame frame{41, 1.5, visionlab::Image(7, 7)};
    std::fill_n(frame.image.data(), frame.image.size_bytes(), std::uint8_t{0});
    for (std::size_t row = 2; row <= 4; ++row)
    {
        for (std::size_t column = 2; column <= 4; ++column)
        {
            std::fill_n(frame.image.data() + row * frame.image.stride_bytes() + column * 3, 3, std::uint8_t{255});
        }
    }
    for (const bool erosion : {true, false})
    {
        visionlab::Pipeline pipeline;
        if (erosion)
        {
            pipeline.add(std::make_unique<visionlab::ErosionStage>());
        }
        else
        {
            pipeline.add(std::make_unique<visionlab::DilationStage>());
        }
        const visionlab::FrameResult result = pipeline.process(frame, true);
        require(result.frame.index == frame.index && result.frame.timestamp_seconds == frame.timestamp_seconds &&
                    result.frame.image.width() == 7 && result.frame.image.height() == 7,
                "Morphology preserves frame metadata and dimensions");
        require(result.snapshots.size() == 2 &&
                    std::equal(frame.image.data(), frame.image.data() + frame.image.size_bytes(), result.snapshots[0].image.data()),
                "Morphology preserves the source mask snapshot");
        for (std::size_t row = 0; row < 7; ++row)
        {
            for (std::size_t column = 0; column < 7; ++column)
            {
                const bool white = erosion ? row == 3 && column == 3 : row >= 1 && row <= 5 && column >= 1 && column <= 5;
                const std::uint8_t expected = white ? 255 : 0;
                for (std::size_t channel = 0; channel < 3; ++channel)
                {
                    const std::size_t offset = row * frame.image.stride_bytes() + column * 3 + channel;
                    require(result.frame.image.data()[offset] == expected && result.snapshots[1].image.data()[offset] == expected,
                            "A 3x3 square erodes to one pixel or dilates to 5x5 in every display channel");
                }
            }
        }
    }
}
void test_hsv_color()
{
    const std::array<std::array<std::uint8_t, 3>, 8> colors{
        {{0, 128, 255}, {0, 64, 128}, {205, 230, 255}, {255, 255, 255}, {0, 255, 0}, {255, 0, 0}, {0, 20, 40}, {0, 0, 255}}};
    const std::array<std::uint8_t, 8> expected{255, 255, 0, 0, 0, 0, 0, 0};
    visionlab::Frame frame{51, 2.0, visionlab::Image(8, 1)};
    for (std::size_t pixel = 0; pixel < colors.size(); ++pixel)
    {
        std::copy(colors[pixel].begin(), colors[pixel].end(), frame.image.data() + pixel * 3);
    }
    const visionlab::app::PipelineCatalog catalog;
    visionlab::Pipeline pipeline = catalog.create("practice-09-hsv-color");
    const visionlab::FrameResult result = pipeline.process(frame, true);
    require(result.frame.index == frame.index && result.frame.timestamp_seconds == frame.timestamp_seconds &&
                result.frame.image.width() == 8 && result.frame.image.height() == 1 && result.snapshots.size() == 2,
            "HSV selection preserves metadata and exposes source plus mask");
    require(std::equal(frame.image.data(), frame.image.data() + frame.image.size_bytes(), result.snapshots[0].image.data()),
            "HSV selection preserves the original color snapshot");
    for (std::size_t pixel = 0; pixel < colors.size(); ++pixel)
    {
        for (std::size_t channel = 0; channel < 3; ++channel)
        {
            require(result.frame.image.data()[pixel * 3 + channel] == expected[pixel],
                    "HSV mask selects vivid orange and rejects other hues, pale colors, and very dark pixels");
        }
    }
}
void test_adaptive_threshold()
{
    // Identical dark strokes at two brightness levels should yield identical local masks.
    std::optional<visionlab::FrameResult> previous;
    for (const std::uint8_t background : {std::uint8_t{80}, std::uint8_t{160}})
    {
        visionlab::Frame frame{61, 2.5, visionlab::Image(21, 21)};
        std::fill_n(frame.image.data(), frame.image.size_bytes(), background);
        for (std::size_t row = 5; row <= 15; ++row)
        {
            std::fill_n(frame.image.data() + row * frame.image.stride_bytes() + 10 * 3, 3, static_cast<std::uint8_t>(background - 40));
        }
        visionlab::Pipeline pipeline;
        pipeline.add(std::make_unique<visionlab::AdaptiveThresholdStage>());
        const visionlab::FrameResult result = pipeline.process(frame, true);
        require(result.frame.index == frame.index && result.frame.timestamp_seconds == frame.timestamp_seconds &&
                    result.frame.image.width() == 21 && result.frame.image.height() == 21 && result.snapshots.size() == 2,
                "Adaptive threshold preserves frame identity and dimensions");
        require(std::equal(frame.image.data(), frame.image.data() + frame.image.size_bytes(), result.snapshots[0].image.data()),
                "Adaptive threshold retains the unmodified source snapshot");
        for (std::size_t channel = 0; channel < 3; ++channel)
        {
            require(result.frame.image.data()[channel] == 255 &&
                        result.frame.image.data()[10 * frame.image.stride_bytes() + 10 * 3 + channel] == 0,
                    "Local threshold retains dark strokes and makes uniform background white in every channel");
        }
        if (previous)
        {
            require(std::equal(result.frame.image.data(), result.frame.image.data() + result.frame.image.size_bytes(),
                               previous->frame.image.data()),
                    "Adding a constant illumination offset preserves the local threshold mask");
        }
        previous = result;
    }
}
void test_otsu_threshold()
{
    // Both classes are below 127 in one case and above it in the other.
    for (const std::uint8_t background : {std::uint8_t{30}, std::uint8_t{150}})
    {
        visionlab::Frame frame{71, 3.0, visionlab::Image(20, 10)};
        for (std::size_t row = 0; row < 10; ++row)
        {
            for (std::size_t column = 0; column < 20; ++column)
            {
                const std::uint8_t value = static_cast<std::uint8_t>(background + (column >= 10 ? 70 : 0));
                std::fill_n(frame.image.data() + row * frame.image.stride_bytes() + column * 3, 3, value);
            }
        }
        const visionlab::app::PipelineCatalog catalog;
        visionlab::Pipeline pipeline = catalog.create("practice-11-otsu-threshold");
        const visionlab::FrameResult result = pipeline.process(frame, true);
        require(result.frame.index == frame.index && result.frame.timestamp_seconds == frame.timestamp_seconds &&
                    result.frame.image.width() == 20 && result.frame.image.height() == 10 && result.snapshots.size() == 4,
                "Otsu pipeline preserves frame identity and exposes preprocessing snapshots");
        require(std::equal(frame.image.data(), frame.image.data() + frame.image.size_bytes(), result.snapshots[0].image.data()),
                "Otsu pipeline preserves the source pixels");
        for (std::size_t channel = 0; channel < 3; ++channel)
        {
            require(result.frame.image.data()[5 * frame.image.stride_bytes() + 3 * 3 + channel] == 0 &&
                        result.frame.image.data()[5 * frame.image.stride_bytes() + 16 * 3 + channel] == 255,
                    "Otsu separates both dim and bright populations without a manually chosen cutoff");
        }
    }
}
void test_morphology_ex_equivalence()
{
    const visionlab::app::PipelineCatalog catalog;
    visionlab::SyntheticSource source(1, 321, 241);
    const visionlab::Frame frame = *source.next();
    const std::array<std::array<const char*, 2>, 2> ids{
        {{"practice-08-opening", "practice-12-opening-ex"}, {"practice-08-closing", "practice-12-closing-ex"}}};
    for (const std::array<const char*, 2>& pair : ids)
    {
        visionlab::Pipeline separate = catalog.create(pair[0]);
        visionlab::Pipeline combined = catalog.create(pair[1]);
        const visionlab::FrameResult expected = separate.process(frame, true);
        const visionlab::FrameResult actual = combined.process(frame, true);
        require(expected.snapshots.size() == 6 && actual.snapshots.size() == 5,
                "Separate morphology exposes two operations; morphologyEx exposes one combined stage");
        require(actual.frame.index == frame.index && actual.frame.timestamp_seconds == frame.timestamp_seconds &&
                    actual.frame.image.width() == frame.image.width() && actual.frame.image.height() == frame.image.height(),
                "Combined morphology preserves frame identity and dimensions");
        require(std::equal(expected.frame.image.data(), expected.frame.image.data() + expected.frame.image.size_bytes(),
                           actual.frame.image.data()),
                "morphologyEx matches separate erosion/dilation with identical settings");
        require(std::equal(frame.image.data(), frame.image.data() + frame.image.size_bytes(), actual.snapshots[0].image.data()),
                "Combined morphology preserves the source snapshot");
    }
}
void test_kernel_shapes()
{
    const std::array<const char*, 5> ids{"practice-13-square", "practice-13-ellipse", "practice-13-cross", "practice-13-horizontal",
                                         "practice-13-vertical"};
    const visionlab::app::PipelineCatalog catalog;
    visionlab::Frame frame{81, 3.5, visionlab::Image(15, 15)};
    std::fill_n(frame.image.data(), frame.image.size_bytes(), std::uint8_t{0});
    std::fill_n(frame.image.data() + 7 * frame.image.stride_bytes() + 7 * 3, 3, std::uint8_t{255});
    for (std::size_t shape = 0; shape < ids.size(); ++shape)
    {
        visionlab::Pipeline pipeline = catalog.create(ids[shape]);
        const visionlab::FrameResult result = pipeline.process(frame, true);
        require(result.snapshots.size() == 3 && result.frame.index == frame.index &&
                    result.frame.timestamp_seconds == frame.timestamp_seconds,
                "Kernel exercise preserves frame identity and exposes threshold and dilation");
        for (int y = 0; y < 15; ++y)
        {
            for (int x = 0; x < 15; ++x)
            {
                const int dx = std::abs(x - 7);
                const int dy = std::abs(y - 7);
                const bool white = shape == 0   ? dx <= 2 && dy <= 2
                                   : shape == 1 ? (dy <= 1 && dx <= 2) || (dy == 2 && dx == 0)
                                   : shape == 2 ? (dx == 0 && dy <= 2) || (dy == 0 && dx <= 2)
                                   : shape == 3 ? dy == 0 && dx <= 4
                                                : dx == 0 && dy <= 4;
                for (std::size_t channel = 0; channel < 3; ++channel)
                {
                    const std::size_t offset = static_cast<std::size_t>(y) * frame.image.stride_bytes() + x * 3 + channel;
                    require(result.frame.image.data()[offset] == (white ? 255 : 0),
                            "An isolated pixel expands into the expected kernel footprint");
                }
            }
        }
    }
}
void test_gradient_direction()
{
    // Linear ramps have exact interior Sobel derivatives: eight times the slope.
    const std::array<std::array<int, 2>, 5> slopes{{{10, 0}, {0, 10}, {-10, 0}, {0, -10}, {0, 0}}};
    const std::array<std::array<int, 3>, 5> expected{{{0, 0, 80}, {0, 80, 40}, {80, 80, 0}, {80, 0, 40}, {0, 0, 0}}};
    for (std::size_t direction = 0; direction < slopes.size(); ++direction)
    {
        visionlab::Frame frame{19, 0.25, visionlab::Image(9, 9)};
        for (int y = 0; y < 9; ++y)
        {
            for (int x = 0; x < 9; ++x)
            {
                const std::uint8_t value = static_cast<std::uint8_t>(128 + slopes[direction][0] * (x - 4) + slopes[direction][1] * (y - 4));
                const std::size_t offset = y * frame.image.stride_bytes() + x * 3;
                std::fill_n(frame.image.data() + offset, 3, value);
            }
        }
        visionlab::Pipeline pipeline;
        pipeline.add(std::make_unique<visionlab::GradientDirectionStage>());
        const visionlab::FrameResult result = pipeline.process(frame, true);
        require(result.frame.index == 19 && result.frame.timestamp_seconds == frame.timestamp_seconds && result.frame.image.width() == 9 &&
                    result.frame.image.height() == 9,
                "Gradient direction preserves frame identity and dimensions");
        require(result.snapshots.size() == 2 && result.snapshots.back().name == "Gradient direction" &&
                    std::equal(frame.image.data(), frame.image.data() + frame.image.size_bytes(), result.snapshots[0].image.data()),
                "Gradient direction preserves its source snapshot");
        const std::size_t center = 4 * result.frame.image.stride_bytes() + 4 * 3;
        for (std::size_t channel = 0; channel < 3; ++channel)
        {
            require(std::abs(static_cast<int>(result.frame.image.data()[center + channel]) - expected[direction][channel]) <= 1,
                    "Signed cardinal gradients map to the expected hue and fixed strength; flat input stays black");
        }
    }
    const visionlab::app::PipelineCatalog catalog;
    visionlab::Pipeline pipeline = catalog.create("practice-14-gradient-direction");
    const visionlab::FrameResult result = pipeline.process({0, 0.0, visionlab::Image(9, 9)}, true);
    require(result.snapshots.size() == 4 && result.snapshots.back().name == "Gradient direction",
            "Practice 14 exposes source, grayscale, Gaussian and gradient direction");
}

void test_sobel_smoothing()
{
    const visionlab::app::PipelineCatalog catalog;
    for (int textured = 0; textured < 2; ++textured)
    {
        visionlab::Frame frame{31, 0.5, visionlab::Image(9, 9)};
        for (int y = 0; y < 9; ++y)
        {
            for (int x = 0; x < 9; ++x)
            {
                // A linear ramp plus alternating-row slope interference.
                const int slope = 10 + (textured ? (y % 2 ? -5 : 5) : 0);
                const std::uint8_t value = static_cast<std::uint8_t>(128 + slope * (x - 4));
                std::fill_n(frame.image.data() + y * frame.image.stride_bytes() + x * 3, 3, value);
            }
        }
        for (int smooth = 0; smooth < 2; ++smooth)
        {
            visionlab::Pipeline pipeline = catalog.create(smooth ? "practice-15-sobel-smoothing" : "practice-15-x-difference");
            const visionlab::FrameResult result = pipeline.process(frame, true);
            require(result.frame.index == 31 && result.frame.timestamp_seconds == frame.timestamp_seconds &&
                        result.frame.image.width() == 9 && result.frame.image.height() == 9 && result.snapshots.size() == 3,
                    "Sobel smoothing pipelines preserve metadata and expose each stage");
            require(std::equal(frame.image.data(), frame.image.data() + frame.image.size_bytes(), result.snapshots[0].image.data()),
                    "Sobel smoothing preserves the source snapshot");
            for (int y = 2; y <= 6; ++y)
            {
                const int expected = !textured || smooth ? 20 : (y % 2 ? 10 : 30);
                const std::size_t offset = y * result.frame.image.stride_bytes() + 4 * 3;
                for (int channel = 0; channel < 3; ++channel)
                {
                    require(result.frame.image.data()[offset + channel] == expected,
                            "Matched gains preserve a clean ramp; vertical smoothing cancels alternating-row interference");
                }
            }
        }
    }
}

void test_canny_walkthrough()
{
    visionlab::Frame frame{43, 1.25, visionlab::Image(400, 240)};
    for (int y = 0; y < 240; ++y)
    {
        for (int x = 0; x < 400; ++x)
        {
            int value = 20;
            if (x >= 60 && x <= 180 && y >= 30 && y <= 210)
            {
                value = y < 90 ? 220 : std::max(100, 220 - 3 * (y - 90));
            }
            else if (x >= 260 && x <= 340 && y >= 130 && y <= 210)
            {
                value = 100;
            }
            std::fill_n(frame.image.data() + y * frame.image.stride_bytes() + x * 3, 3, static_cast<std::uint8_t>(value));
        }
    }
    const visionlab::app::PipelineCatalog catalog;
    visionlab::Pipeline pipeline = catalog.create("practice-16-canny-walkthrough");
    const visionlab::FrameResult result = pipeline.process(frame, true);
    require(result.snapshots.size() == 8 && result.frame.index == 43 && result.frame.timestamp_seconds == frame.timestamp_seconds,
            "Canny walkthrough exposes eight views and preserves frame identity");
    require(std::equal(frame.image.data(), frame.image.data() + frame.image.size_bytes(), result.snapshots[0].image.data()),
            "Canny walkthrough preserves the source snapshot");
    for (const int edge_x : {60, 260})
    {
        int weak_count = 0;
        int kept_count = 0;
        int reference_count = 0;
        int magnitude_count = 0;
        int thin_count = 0;
        for (int x = edge_x - 4; x <= edge_x + 4; ++x)
        {
            const std::size_t offset = 175 * frame.image.stride_bytes() + x * 3;
            const std::uint8_t* classified = result.snapshots[5].image.data() + offset;
            weak_count += classified[0] == 0 && classified[1] == 165 && classified[2] == 255;
            kept_count += result.snapshots[6].image.data()[offset] != 0;
            reference_count += result.snapshots[7].image.data()[offset] != 0;
            magnitude_count += result.snapshots[3].image.data()[offset] != 0;
            thin_count += result.snapshots[4].image.data()[offset] != 0;
        }
        require(weak_count == 1 && thin_count == 1 && magnitude_count > thin_count,
                "Non-maximum suppression thins the weak edge to one classified candidate");
        require(kept_count == (edge_x == 60 ? 1 : 0) && reference_count == kept_count,
                "Hysteresis retains long connected weak chains and rejects a disconnected weak edge");
    }
    const visionlab::FrameResult blank = pipeline.process({44, 1.5, visionlab::Image(13, 11)}, true);
    for (std::size_t index = 3; index < blank.snapshots.size(); ++index)
    {
        const visionlab::Image& image = blank.snapshots[index].image;
        require(std::all_of(image.data(), image.data() + image.size_bytes(),
                            [](std::uint8_t pixel)
                            {
                                return pixel == 0;
                            }),
                "A new flat frame and changed dimensions do not retain previous Canny data");
    }
    require(result.snapshots[5].image.width() == 400, "Later processing leaves earlier snapshots owned and intact");
}

void test_hsv_cleanup()
{
    visionlab::Frame frame{51, 2.0, visionlab::Image(160, 100)};
    for (int y = 0; y < 100; ++y)
    {
        for (int x = 0; x < 160; ++x)
        {
            const bool body = x >= 20 && x <= 80 && y >= 20 && y <= 75;
            const bool hole = x >= 34 && x <= 35 && y >= 40 && y <= 41;
            const bool speck = x >= 100 && x <= 101 && y >= 12 && y <= 13;
            const bool patch = x >= 110 && x <= 114 && y >= 65 && y <= 69;
            std::array<std::uint8_t, 3> color{30, 30, 30};
            if ((body && x != 50 && x != 51 && !hole) || speck || patch)
            {
                color = {0, 140, 255};
            }
            if (x >= 105 && x <= 140 && y >= 25 && y <= 45)
            {
                color = {255, 80, 0};
            }
            std::copy(color.begin(), color.end(), frame.image.data() + y * frame.image.stride_bytes() + x * 3);
        }
    }
    const visionlab::app::PipelineCatalog catalog;
    visionlab::Pipeline pipeline = catalog.create("practice-17-hsv-cleanup");
    const visionlab::FrameResult result = pipeline.process(frame, true);
    require(result.snapshots.size() == 4 && result.frame.index == 51 && result.frame.timestamp_seconds == frame.timestamp_seconds,
            "HSV cleanup exposes source, mask, opening and closing with original frame metadata");
    require(std::equal(frame.image.data(), frame.image.data() + frame.image.size_bytes(), result.snapshots[0].image.data()),
            "HSV cleanup preserves the color source snapshot");
    const std::function<std::uint8_t(std::size_t, int, int)> pixel = [&result](std::size_t stage, int x, int y)
    {
        const visionlab::Image& image = result.snapshots[stage].image;
        return image.data()[y * image.stride_bytes() + x * 3];
    };
    require(pixel(1, 100, 12) == 255 && pixel(2, 100, 12) == 0 && pixel(3, 100, 12) == 0, "Opening removes selected tiny orange spots");
    require(pixel(1, 34, 40) == 0 && pixel(2, 34, 40) == 0 && pixel(3, 34, 40) == 255, "Closing fills the small mask hole");
    require(pixel(2, 50, 55) == 0 && pixel(3, 50, 55) == 255, "Closing bridges the narrow gap after opening");
    for (std::size_t stage = 1; stage < result.snapshots.size(); ++stage)
    {
        require(pixel(stage, 112, 67) == 255 && pixel(stage, 120, 35) == 0 && pixel(stage, 25, 25) == 255,
                "Larger orange structures survive; blue is rejected");
        const visionlab::Image& image = result.snapshots[stage].image;
        for (std::size_t offset = 0; offset < image.size_bytes(); offset += 3)
        {
            require((image.data()[offset] == 0 || image.data()[offset] == 255) && image.data()[offset] == image.data()[offset + 1] &&
                        image.data()[offset] == image.data()[offset + 2],
                    "Each cleanup stage stays a binary BGR mask");
        }
    }
}

void test_connected_components()
{
    visionlab::Frame frame{61, 2.5, visionlab::Image(20, 15)};
    for (int y = 0; y < 15; ++y)
    {
        for (int x = 0; x < 20; ++x)
        {
            const bool ring = x >= 2 && x <= 6 && y >= 2 && y <= 6 && (x == 2 || x == 6 || y == 2 || y == 6);
            const bool diagonal = (x == 12 && y == 9) || (x == 13 && y == 10);
            std::fill_n(frame.image.data() + y * frame.image.stride_bytes() + x * 3, 3, ring || diagonal ? 255 : 0);
        }
    }
    visionlab::Pipeline pipeline;
    pipeline.add(std::make_unique<visionlab::ConnectedComponentsStage>());
    const visionlab::FrameResult result = pipeline.process(frame, true);
    require(result.boxes.size() == 2 && result.snapshots.size() == 2 && result.snapshots[0].boxes.empty() &&
                result.snapshots[1].boxes.size() == 2 && result.frame.index == 61 &&
                result.frame.timestamp_seconds == frame.timestamp_seconds,
            "Components exclude background and attach owned boxes only to the processed snapshot");
    for (const visionlab::BoxOverlay& box : result.boxes)
    {
        if (box.x == 2)
        {
            require(box.y == 2 && box.width == 5 && box.height == 5 && box.label.find("area=16") != std::string::npos,
                    "Ring area excludes its hole, while its bounding box encloses it");
        }
        else
        {
            require(box.x == 12 && box.y == 9 && box.width == 2 && box.height == 2 && box.label.find("area=2") != std::string::npos,
                    "Eight-connectivity groups diagonally touching pixels");
        }
    }
    require(std::equal(frame.image.data(), frame.image.data() + frame.image.size_bytes(), result.snapshots[0].image.data()),
            "Component visualization does not modify the saved binary mask");
    require(result.frame.image.data()[4 * frame.image.stride_bytes() + 4 * 3] == 0, "The ring's background hole remains black");
    const visionlab::FrameResult blank = pipeline.process({62, 3.0, visionlab::Image(8, 8)}, true);
    require(blank.boxes.empty() && result.snapshots[1].boxes.size() == 2, "Blank frames have no components or stale boxes");
    const visionlab::app::PipelineCatalog catalog;
    visionlab::Pipeline registered = catalog.create("practice-18-connected-components");
    visionlab::Frame orange{0, 0.0, visionlab::Image(20, 20)};
    for (int y = 5; y < 15; ++y)
    {
        for (int x = 5; x < 15; ++x)
        {
            const std::size_t offset = y * orange.image.stride_bytes() + x * 3;
            orange.image.data()[offset + 1] = 140;
            orange.image.data()[offset + 2] = 255;
        }
    }
    const visionlab::FrameResult composed = registered.process(orange, true);
    require(composed.snapshots.size() == 5 && composed.boxes.size() == 1 && composed.boxes[0].label.find("area=100") != std::string::npos,
            "The HSV cleanup pipeline passes its mask to component extraction");
}

void test_component_area_filter()
{
    // A 10x10 filled square (area 100) and a 12x12 outline (area 44, box area 144).
    visionlab::Frame frame{71, 3.5, visionlab::Image(40, 20)};
    for (int y = 0; y < 20; ++y)
    {
        for (int x = 0; x < 40; ++x)
        {
            const bool square = x >= 2 && x < 12 && y >= 2 && y < 12;
            const bool ring = x >= 22 && x <= 33 && y >= 2 && y <= 13 && (x == 22 || x == 33 || y == 2 || y == 13);
            std::fill_n(frame.image.data() + y * frame.image.stride_bytes() + x * 3, 3, square || ring ? 255 : 0);
        }
    }
    for (const int minimum : {44, 45, 100, 101})
    {
        visionlab::Pipeline pipeline;
        pipeline.add(std::make_unique<visionlab::ConnectedComponentsStage>(minimum));
        const visionlab::FrameResult result = pipeline.process(frame, true);
        const std::size_t expected_count = minimum == 44 ? 2 : (minimum <= 100 ? 1 : 0);
        require(result.boxes.size() == expected_count && result.snapshots.back().boxes.size() == expected_count,
                "Area cutoff is inclusive and uses pixel area rather than bounding-box area");
        require(result.frame.index == 71 && result.frame.timestamp_seconds == frame.timestamp_seconds &&
                    std::equal(frame.image.data(), frame.image.data() + frame.image.size_bytes(), result.snapshots[0].image.data()),
                "Area filtering preserves frame metadata and the source mask snapshot");
        for (int y = 0; y < 20; ++y)
        {
            for (int x = 0; x < 40; ++x)
            {
                const std::size_t offset = y * frame.image.stride_bytes() + x * 3;
                const bool expected = frame.image.data()[offset] != 0 && (x < 20 ? minimum <= 100 : minimum <= 44);
                const bool colored =
                    result.frame.image.data()[offset] || result.frame.image.data()[offset + 1] || result.frame.image.data()[offset + 2];
                require(colored == expected, "Kept component shapes are unchanged; rejected components are entirely black");
            }
        }
    }
    for (const int invalid : {0, -1})
    {
        bool rejected = false;
        try
        {
            visionlab::ConnectedComponentsStage stage(invalid);
        }
        catch (const std::invalid_argument&)
        {
            rejected = true;
        }
        require(rejected, "Nonpositive minimum areas are rejected");
    }
    const visionlab::app::PipelineCatalog catalog;
    visionlab::Pipeline registered = catalog.create("practice-19-component-area");
    visionlab::Frame orange{0, 0.0, visionlab::Image(40, 20)};
    for (int y = 2; y < 12; ++y)
    {
        for (int x = 2; x < 32; ++x)
        {
            if (x < 12 || (x >= 22 && x < 27 && y < 7))
            {
                const std::size_t offset = y * orange.image.stride_bytes() + x * 3;
                orange.image.data()[offset + 1] = 140;
                orange.image.data()[offset + 2] = 255;
            }
        }
    }
    const visionlab::FrameResult composed = registered.process(orange, true);
    require(composed.snapshots.size() == 5 && composed.boxes.size() == 1 && composed.boxes[0].label.find("area=100") != std::string::npos,
            "Practice 19 keeps the 100-pixel region and rejects the separate 25-pixel region");
}

void test_external_contours()
{
    // An isolated solid rectangle and a hollow rectangle with a nested foreground island.
    visionlab::Frame frame{81, 4.0, visionlab::Image(30, 20)};
    for (int y = 0; y < 20; ++y)
    {
        for (int x = 0; x < 30; ++x)
        {
            const bool rectangle = x >= 2 && x <= 7 && y >= 3 && y <= 10;
            const bool outer = x >= 14 && x <= 26 && y >= 2 && y <= 16;
            const bool hole = x >= 17 && x <= 23 && y >= 5 && y <= 13;
            const bool island = x == 20 && y == 9;
            std::fill_n(frame.image.data() + y * frame.image.stride_bytes() + x * 3, 3, rectangle || (outer && !hole) || island ? 255 : 0);
        }
    }
    visionlab::Pipeline pipeline;
    pipeline.add(std::make_unique<visionlab::ContoursStage>());
    const visionlab::FrameResult result = pipeline.process(frame, true);
    require(result.snapshots.size() == 2 && result.frame.index == 81 && result.frame.timestamp_seconds == frame.timestamp_seconds &&
                result.boxes.empty(),
            "Contours preserve frame identity and do not overlay bounding boxes");
    require(std::equal(frame.image.data(), frame.image.data() + frame.image.size_bytes(), result.snapshots[0].image.data()),
            "Contours preserve the source mask snapshot");
    for (int y = 0; y < 20; ++y)
    {
        for (int x = 0; x < 30; ++x)
        {
            const bool rectangle = x >= 2 && x <= 7 && y >= 3 && y <= 10 && (x == 2 || x == 7 || y == 3 || y == 10);
            const bool ring = x >= 14 && x <= 26 && y >= 2 && y <= 16 && (x == 14 || x == 26 || y == 2 || y == 16);
            const std::size_t offset = y * frame.image.stride_bytes() + x * 3;
            require(result.frame.image.data()[offset] == (rectangle || ring ? 80 : 0) &&
                        result.frame.image.data()[offset + 1] == (rectangle || ring ? 230 : 0) &&
                        result.frame.image.data()[offset + 2] == (rectangle || ring ? 255 : 0),
                    "External contours draw complete outer outlines but omit interiors, holes and nested islands");
        }
    }
    const visionlab::FrameResult blank = pipeline.process({82, 4.5, visionlab::Image(9, 7)}, true);
    require(std::all_of(blank.frame.image.data(), blank.frame.image.data() + blank.frame.image.size_bytes(),
                        [](std::uint8_t value)
                        {
                            return value == 0;
                        }),
            "An empty mask has no contour pixels or stale output");
    const visionlab::app::PipelineCatalog catalog;
    visionlab::Pipeline registered = catalog.create("practice-20-contours");
    visionlab::Frame orange{0, 0.0, visionlab::Image(20, 20)};
    for (int y = 5; y < 15; ++y)
    {
        for (int x = 5; x < 15; ++x)
        {
            const std::size_t offset = y * orange.image.stride_bytes() + x * 3;
            orange.image.data()[offset + 1] = 140;
            orange.image.data()[offset + 2] = 255;
        }
    }
    const visionlab::FrameResult composed = registered.process(orange, true);
    require(composed.snapshots.size() == 5 && composed.frame.image.data()[5 * orange.image.stride_bytes() + 5 * 3] == 80 &&
                composed.frame.image.data()[10 * orange.image.stride_bytes() + 10 * 3] == 0,
            "Practice 20 extracts outlines directly from its cleaned HSV mask");
}

void test_contour_measurements()
{
    visionlab::Frame frame{91, 5.0, visionlab::Image(40, 20)};
    for (int y = 2; y <= 11; ++y)
    {
        for (int x = 2; x <= 31; ++x)
        {
            const bool square = x <= 11;
            const bool ring = x >= 22 && (x == 22 || x == 31 || y == 2 || y == 11);
            if (square || ring)
            {
                std::fill_n(frame.image.data() + y * frame.image.stride_bytes() + x * 3, 3, 255);
            }
        }
    }
    std::fill_n(frame.image.data() + 16 * frame.image.stride_bytes() + 36 * 3, 3, 255);
    visionlab::Pipeline pipeline;
    pipeline.add(std::make_unique<visionlab::ContoursStage>(true));
    const visionlab::FrameResult result = pipeline.process(frame, true);
    require(result.boxes.size() == 3 && result.snapshots.size() == 2 && result.snapshots[0].boxes.empty() &&
                result.snapshots[1].boxes.size() == 3 && result.frame.index == 91 &&
                result.frame.timestamp_seconds == frame.timestamp_seconds,
            "Contour measurements expose boxes in the final snapshot and preserve frame metadata");
    for (const visionlab::BoxOverlay& box : result.boxes)
    {
        if (box.x == 36)
        {
            require(box.y == 16 && box.width == 1 && box.height == 1 && box.label == "A=0.0 px^2\nP=0.0 px",
                    "A point contour has zero geometric area and length but a one-pixel box");
        }
        else
        {
            require((box.x == 2 || box.x == 22) && box.y == 2 && box.width == 10 && box.height == 10 &&
                        box.label == "A=81.0 px^2\nP=36.0 px",
                    "Solid and hollow 10x10 shapes have the same external contour measurements; closure includes all four sides");
        }
    }
    require(std::equal(frame.image.data(), frame.image.data() + frame.image.size_bytes(), result.snapshots[0].image.data()),
            "Measurement overlays preserve the original source mask");
    const visionlab::FrameResult blank = pipeline.process({92, 5.5, visionlab::Image(5, 7)}, true);
    require(blank.boxes.empty() && result.snapshots[1].boxes.size() == 3, "Empty frames do not retain old measurement overlays");
    const visionlab::app::PipelineCatalog catalog;
    visionlab::Pipeline registered = catalog.create("practice-21-contour-measurements");
    visionlab::Frame orange{0, 0.0, visionlab::Image(20, 20)};
    for (int y = 5; y < 15; ++y)
    {
        for (int x = 5; x < 15; ++x)
        {
            const std::size_t offset = y * orange.image.stride_bytes() + x * 3;
            orange.image.data()[offset + 1] = 140;
            orange.image.data()[offset + 2] = 255;
        }
    }
    const visionlab::FrameResult composed = registered.process(orange, true);
    require(composed.snapshots.size() == 5 && composed.boxes.size() == 1 && composed.boxes[0].label == "A=81.0 px^2\nP=36.0 px",
            "Practice 21 measures the cleaned HSV mask");
}

void test_contour_hierarchy()
{
    visionlab::Frame frame{101, 6.0, visionlab::Image(50, 45)};
    for (int y = 0; y < 45; ++y)
    {
        for (int x = 0; x < 50; ++x)
        {
            const bool outer = x >= 2 && x <= 40 && y >= 2 && y <= 40;
            const bool hole = x >= 8 && x <= 34 && y >= 8 && y <= 34;
            const bool island = x >= 14 && x <= 28 && y >= 14 && y <= 28;
            const bool inner_hole = x >= 19 && x <= 23 && y >= 19 && y <= 23;
            const bool separate = x == 46 && y == 2;
            std::fill_n(frame.image.data() + y * frame.image.stride_bytes() + x * 3, 3,
                        (outer && !hole) || (island && !inner_hole) || separate ? 255 : 0);
        }
    }
    visionlab::Pipeline pipeline;
    pipeline.add(std::make_unique<visionlab::ContourHierarchyStage>());
    const visionlab::FrameResult result = pipeline.process(frame, true);
    require(result.boxes.size() == 5 && result.snapshots.size() == 2 && result.snapshots[0].boxes.empty() &&
                result.snapshots[1].boxes.size() == 5 && result.frame.index == 101 &&
                result.frame.timestamp_seconds == frame.timestamp_seconds,
            "Contour tree exposes nested holes, islands and separate roots with owned overlays");
    std::array<int, 4> nested_indices{-1, -1, -1, -1};
    const std::array<float, 4> left{2, 7, 14, 18};
    for (std::size_t index = 0; index < result.boxes.size(); ++index)
    {
        for (std::size_t depth = 0; depth < left.size(); ++depth)
        {
            if (result.boxes[index].x == left[depth])
            {
                nested_indices[depth] = static_cast<int>(index);
            }
        }
    }
    for (std::size_t depth = 0; depth < nested_indices.size(); ++depth)
    {
        require(nested_indices[depth] >= 0, "Every expected nesting level was found");
        const int index = nested_indices[depth];
        const int parent = depth == 0 ? -1 : nested_indices[depth - 1];
        const std::string role = depth % 2 != 0 ? "hole" : (depth == 0 ? "outer" : "island");
        const std::string expected =
            "#" + std::to_string(index) + " " + role + " d=" + std::to_string(depth) + "\nparent=" + std::to_string(parent);
        require(result.boxes[index].label == expected, "Labels identify the actual parent chain, without assuming contour order");
    }
    for (const visionlab::BoxOverlay& box : result.boxes)
    {
        if (box.x == 46)
        {
            require(box.label.find("outer d=0\nparent=-1") != std::string::npos, "The independent contour is another root");
        }
    }
    const std::size_t outer_pixel = 10 * frame.image.stride_bytes() + 2 * 3;
    const std::size_t hole_pixel = 10 * frame.image.stride_bytes() + 7 * 3;
    const std::size_t island_pixel = 20 * frame.image.stride_bytes() + 14 * 3;
    require(result.frame.image.data()[outer_pixel] == 80 && result.frame.image.data()[hole_pixel] == 255 &&
                result.frame.image.data()[island_pixel] == 220,
            "Outer, hole and island contours receive distinct depth colors");
    require(std::equal(frame.image.data(), frame.image.data() + frame.image.size_bytes(), result.snapshots[0].image.data()),
            "Hierarchy visualization preserves the input snapshot");
    const visionlab::FrameResult blank = pipeline.process({102, 6.5, visionlab::Image(9, 7)}, true);
    require(blank.boxes.empty() && std::all_of(blank.frame.image.data(), blank.frame.image.data() + blank.frame.image.size_bytes(),
                                               [](std::uint8_t value)
                                               {
                                                   return value == 0;
                                               }),
            "Empty masks produce no stale contours or hierarchy overlays");
    const visionlab::app::PipelineCatalog catalog;
    visionlab::Pipeline registered = catalog.create("practice-22-contour-hierarchy");
    visionlab::Frame orange{0, 0.0, visionlab::Image(20, 20)};
    for (int y = 5; y < 15; ++y)
    {
        for (int x = 5; x < 15; ++x)
        {
            const std::size_t offset = y * orange.image.stride_bytes() + x * 3;
            orange.image.data()[offset + 1] = 140;
            orange.image.data()[offset + 2] = 255;
        }
    }
    const visionlab::FrameResult composed = registered.process(orange, true);
    require(composed.snapshots.size() == 5 && composed.boxes.size() == 1 && composed.boxes[0].label == "#0 outer d=0\nparent=-1",
            "Practice 22 extracts hierarchy from the cleaned HSV mask");
}

void test_rotated_rectangle()
{
    // A diamond has a 21x21 pixel axis box, but its minimum rectangle follows the four diagonal sides.
    visionlab::Frame frame{111, 7.0, visionlab::Image(41, 41)};
    for (int y = 0; y < 41; ++y)
    {
        for (int x = 0; x < 41; ++x)
        {
            std::fill_n(frame.image.data() + y * frame.image.stride_bytes() + x * 3, 3,
                        std::abs(x - 20) + std::abs(y - 20) <= 10 ? 255 : 0);
        }
    }
    visionlab::Pipeline pipeline;
    pipeline.add(std::make_unique<visionlab::RotatedRectangleStage>());
    const visionlab::FrameResult result = pipeline.process(frame, true);
    require(result.boxes.size() == 1 && result.boxes[0].x == 10 && result.boxes[0].y == 10 && result.boxes[0].width == 21 &&
                result.boxes[0].height == 21 && result.boxes[0].label.find("rot 14.1x14.1") == 0,
            "The diagonal box has side length sqrt(200), smaller than its axis-aligned enclosure");
    for (const std::array<int, 2>& point : std::array<std::array<int, 2>, 4>{{{15, 15}, {25, 15}, {25, 25}, {15, 25}}})
    {
        const std::size_t offset = point[1] * frame.image.stride_bytes() + point[0] * 3;
        require(result.frame.image.data()[offset] == 255 && result.frame.image.data()[offset + 1] == 230 &&
                    result.frame.image.data()[offset + 2] == 40,
                "All four rotated sides are drawn, including the closing segment");
    }
    require(result.frame.image.data()[10 * frame.image.stride_bytes() + 10 * 3] == 0 &&
                result.frame.image.data()[20 * frame.image.stride_bytes() + 20 * 3] == 45,
            "The empty axis-box corner stays black and the actual region remains visible in dim gray");
    require(result.frame.index == 111 && result.frame.timestamp_seconds == frame.timestamp_seconds && result.snapshots.size() == 2 &&
                result.snapshots[0].boxes.empty() && result.snapshots[1].boxes.size() == 1 &&
                std::equal(frame.image.data(), frame.image.data() + frame.image.size_bytes(), result.snapshots[0].image.data()),
            "Rotated fitting preserves metadata and owned mask snapshots");
    visionlab::Frame point{112, 7.5, visionlab::Image(9, 9)};
    std::fill_n(point.image.data() + 4 * point.image.stride_bytes() + 4 * 3, 3, 255);
    const visionlab::FrameResult degenerate = pipeline.process(point, true);
    require(degenerate.boxes.size() == 1 && degenerate.boxes[0].width == 1 && degenerate.boxes[0].height == 1 &&
                degenerate.boxes[0].label.find("rot 0.0x0.0") == 0,
            "Single points produce a drawable degenerate rotated rectangle");
    const visionlab::FrameResult blank = pipeline.process({113, 8.0, visionlab::Image(7, 5)}, true);
    require(blank.boxes.empty() && std::all_of(blank.frame.image.data(), blank.frame.image.data() + blank.frame.image.size_bytes(),
                                               [](std::uint8_t value)
                                               {
                                                   return value == 0;
                                               }),
            "Empty frames have no stale fitted boxes");
    const visionlab::app::PipelineCatalog catalog;
    visionlab::Pipeline registered = catalog.create("practice-23-rotated-rectangle");
    visionlab::Frame orange{0, 0.0, visionlab::Image(20, 20)};
    for (int y = 5; y < 15; ++y)
    {
        for (int x = 5; x < 15; ++x)
        {
            const std::size_t offset = y * orange.image.stride_bytes() + x * 3;
            orange.image.data()[offset + 1] = 140;
            orange.image.data()[offset + 2] = 255;
        }
    }
    const visionlab::FrameResult composed = registered.process(orange, true);
    require(composed.snapshots.size() == 5 && composed.boxes.size() == 1 && composed.boxes[0].label.find("rot 9.0x9.0") == 0,
            "Practice 23 fits rectangles to the cleaned HSV mask");
}

void test_moments_centroid()
{
    visionlab::Pipeline pipeline;
    pipeline.add(std::make_unique<visionlab::MomentsStage>());
    for (const int shift : {0, 5})
    {
        // Triangle vertices (10,10), (70,10), (10,70): area 1800, centroid (30,30), box center (40,40).
        visionlab::Frame frame{121, 8.0, visionlab::Image(90, 90)};
        for (int y = 10 + shift; y <= 70 + shift; ++y)
        {
            for (int x = 10 + shift; x <= 70 + shift; ++x)
            {
                if (x + y <= 80 + 2 * shift)
                {
                    std::fill_n(frame.image.data() + y * frame.image.stride_bytes() + x * 3, 3, 255);
                }
            }
        }
        const visionlab::FrameResult result = pipeline.process(frame, true);
        const std::string center = shift == 0 ? "30.0,30.0" : "35.0,35.0";
        require(result.boxes.size() == 1 && result.boxes[0].label == "C=(" + center + ")\nm00=1800.0",
                "Contour centroid follows the known triangle geometry and translates without changing area");
        const std::size_t centroid = (30 + shift) * frame.image.stride_bytes() + (30 + shift) * 3;
        const std::size_t box_circle = (40 + shift) * frame.image.stride_bytes() + (46 + shift) * 3;
        require(result.frame.image.data()[centroid] == 40 && result.frame.image.data()[centroid + 2] == 255 &&
                    result.frame.image.data()[box_circle] == 255 && result.frame.image.data()[box_circle + 2] == 40,
                "Yellow centroid and cyan box-center markers are drawn at distinct expected positions");
        require(result.frame.index == 121 && result.frame.timestamp_seconds == frame.timestamp_seconds && result.snapshots.size() == 2 &&
                    result.snapshots[0].boxes.empty() && result.snapshots[1].boxes.size() == 1 &&
                    std::equal(frame.image.data(), frame.image.data() + frame.image.size_bytes(), result.snapshots[0].image.data()),
                "Moments visualization preserves frame metadata and the owned source mask");
    }
    for (const int length : {1, 7})
    {
        visionlab::Frame frame{122, 8.5, visionlab::Image(21, 21)};
        for (int x = 7; x < 7 + length; ++x)
        {
            std::fill_n(frame.image.data() + 10 * frame.image.stride_bytes() + x * 3, 3, 255);
        }
        const visionlab::FrameResult result = pipeline.process(frame, true);
        require(result.boxes.size() == 1 && result.boxes[0].label == "centroid undefined\nm00=0.0",
                "Point and line contours safely report undefined area centroids");
    }
    const visionlab::FrameResult blank = pipeline.process({123, 9.0, visionlab::Image(7, 5)}, true);
    require(blank.boxes.empty() && std::all_of(blank.frame.image.data(), blank.frame.image.data() + blank.frame.image.size_bytes(),
                                               [](std::uint8_t value)
                                               {
                                                   return value == 0;
                                               }),
            "Blank frames do not retain moments markers or labels");
    const visionlab::app::PipelineCatalog catalog;
    visionlab::Pipeline registered = catalog.create("practice-24-moments");
    visionlab::Frame orange{0, 0.0, visionlab::Image(30, 30)};
    for (int y = 5; y < 15; ++y)
    {
        for (int x = 5; x < 15; ++x)
        {
            const std::size_t offset = y * orange.image.stride_bytes() + x * 3;
            orange.image.data()[offset + 1] = 140;
            orange.image.data()[offset + 2] = 255;
        }
    }
    const visionlab::FrameResult composed = registered.process(orange, true);
    require(composed.snapshots.size() == 5 && composed.boxes.size() == 1 && composed.boxes[0].label == "C=(9.5,9.5)\nm00=81.0",
            "Practice 24 measures contour area moments, not binary pixel-count moments");
}

void test_classical_detector()
{
    visionlab::Frame frame{131, 10.0, visionlab::Image(520, 320)};
    for (int y = 0; y < 320; ++y)
    {
        for (int x = 0; x < 520; ++x)
        {
            const bool disk = (x - 85) * (x - 85) + (y - 90) * (y - 90) <= 48 * 48;
            const bool square = x >= 200 && x <= 280 && y >= 50 && y <= 130;
            const double ex = (x - 420) / 65.0;
            const double ey = (y - 90) / 22.0;
            const bool ellipse = ex * ex + ey * ey <= 1.0;
            const bool small = (x - 85) * (x - 85) + (y - 245) * (y - 245) <= 7 * 7;
            const int radius = (x - 240) * (x - 240) + (y - 245) * (y - 245);
            const bool ring = radius >= 24 * 24 && radius <= 48 * 48;
            const bool green = (x - 420) * (x - 420) + (y - 245) * (y - 245) <= 48 * 48;
            const std::array<std::uint8_t, 3> color =
                green ? std::array<std::uint8_t, 3>{0, 200, 0}
                      : (disk || square || ellipse || small || ring ? std::array<std::uint8_t, 3>{0, 140, 255}
                                                                    : std::array<std::uint8_t, 3>{25, 25, 25});
            std::copy(color.begin(), color.end(), frame.image.data() + y * frame.image.stride_bytes() + x * 3);
        }
    }
    const visionlab::app::PipelineCatalog catalog;
    visionlab::Pipeline pipeline = catalog.create("practice-25-classical-detector");
    const visionlab::FrameResult result = pipeline.process(frame, true);
    require(result.snapshots.size() == 6 && result.snapshots[4].boxes.size() == 5 && result.boxes.size() == 1,
            "Detector exposes all stages, excludes green from candidates, and accepts exactly one target");
    int passed = 0;
    int small = 0;
    int holes = 0;
    int not_round = 0;
    for (const visionlab::BoxOverlay& box : result.snapshots[4].boxes)
    {
        passed += box.label.find("PASS") == 0;
        small += box.label.find("too small") == 0;
        holes += box.label.find("has hole") == 0;
        not_round += box.label.find("not round") == 0;
    }
    require(passed == 1 && small == 1 && holes == 1 && not_round == 2, "Each distractor receives the expected rejection reason");
    const visionlab::BoxOverlay& detection = result.boxes[0];
    require(detection.x < 85 && detection.x + detection.width > 85 && detection.y < 90 && detection.y + detection.height > 90 &&
                detection.label.find("orange round object") == 0,
            "The accepted box encloses the intended disk");
    const std::size_t centroid = 90 * frame.image.stride_bytes() + 85 * 3;
    require(result.frame.image.data()[centroid] == 255 && result.frame.image.data()[centroid + 1] == 255 &&
                result.frame.image.data()[centroid + 2] == 255,
            "Detection draws the moment centroid on the source");
    const std::size_t square_center = 90 * frame.image.stride_bytes() + 240 * 3;
    require(result.frame.image.data()[square_center + 1] == 140 && result.frame.image.data()[square_center + 2] == 255 &&
                std::equal(frame.image.data(), frame.image.data() + frame.image.size_bytes(), result.snapshots[4].image.data()) &&
                result.frame.index == 131 && result.frame.timestamp_seconds == frame.timestamp_seconds,
            "Candidate and detection views restore original colors and preserve frame metadata");
    visionlab::Pipeline independent = catalog.create("practice-25-classical-detector");
    const visionlab::FrameResult headless = independent.process(frame, false);
    require(
        headless.snapshots.empty() && headless.boxes.size() == 1 &&
            std::equal(result.frame.image.data(), result.frame.image.data() + result.frame.image.size_bytes(), headless.frame.image.data()),
        "Headless and inspected detector outputs agree without relying on source snapshots");
    const visionlab::FrameResult blank = pipeline.process({132, 10.5, visionlab::Image(13, 11)}, true);
    require(blank.boxes.empty() && blank.snapshots[4].boxes.empty() && result.snapshots[4].boxes.size() == 5 &&
                std::all_of(blank.frame.image.data(), blank.frame.image.data() + blank.frame.image.size_bytes(),
                            [](std::uint8_t pixel)
                            {
                                return pixel == 0;
                            }),
            "New empty frames clear working state and restore the new source at its new dimensions");
}

void test_harris_pipeline()
{
    const visionlab::app::PipelineCatalog catalog;
    visionlab::Pipeline pipeline = catalog.create("practice-26-harris");
    visionlab::Frame frame{141, 11.0, visionlab::Image(90, 80)};
    for (int y = 20; y <= 60; ++y)
    {
        for (int x = 20; x <= 70; ++x)
        {
            std::fill_n(frame.image.data() + y * frame.image.stride_bytes() + x * 3, 3, 230);
        }
    }
    const visionlab::FrameResult result = pipeline.process(frame, true);
    require(result.snapshots.size() == 4 && result.boxes.size() == 4 && result.frame.index == 141 &&
                result.frame.timestamp_seconds == frame.timestamp_seconds,
            "A clean rectangle yields four selected Harris corners and all intermediate views");
    for (const std::array<int, 2>& expected : std::array<std::array<int, 2>, 4>{{{20, 20}, {70, 20}, {20, 60}, {70, 60}}})
    {
        int nearby = 0;
        for (const visionlab::BoxOverlay& box : result.boxes)
        {
            nearby += std::abs(box.x + 3 - expected[0]) <= 2 && std::abs(box.y + 3 - expected[1]) <= 2;
        }
        require(nearby == 1, "Each geometric corner has exactly one nearby marker after suppression");
    }
    const visionlab::Image& response = result.snapshots[2].image;
    const std::size_t edge = 40 * response.stride_bytes() + 20 * 3;
    require(response.data()[edge] > 0 && response.data()[edge + 2] == 0,
            "The signed Harris heatmap shows a straight edge as negative blue");
    require(std::equal(frame.image.data(), frame.image.data() + frame.image.size_bytes(), result.frame.image.data()) &&
                result.snapshots[1].boxes.empty() && result.snapshots[2].boxes.empty(),
            "The corner view restores source pixels; heatmap and grayscale have no stale markers");
    visionlab::Pipeline independent = catalog.create("practice-26-harris");
    const visionlab::FrameResult headless = independent.process(frame, false);
    require(headless.snapshots.empty() && headless.boxes.size() == result.boxes.size(), "Harris works without captured snapshots");
    for (std::size_t index = 0; index < result.boxes.size(); ++index)
    {
        require(headless.boxes[index].x == result.boxes[index].x && headless.boxes[index].y == result.boxes[index].y,
                "Independent Harris factories select deterministic points");
    }
    visionlab::Frame straight{142, 11.5, visionlab::Image(50, 50)};
    for (int y = 0; y < 50; ++y)
    {
        for (int x = 25; x < 50; ++x)
        {
            std::fill_n(straight.image.data() + y * straight.image.stride_bytes() + x * 3, 3, 255);
        }
    }
    require(pipeline.process(straight, false).boxes.empty(), "An uninterrupted straight edge does not produce Harris corners");
    const visionlab::FrameResult blank = pipeline.process({143, 12.0, visionlab::Image(7, 5)}, true);
    require(blank.boxes.empty() &&
                std::all_of(blank.snapshots[2].image.data(), blank.snapshots[2].image.data() + blank.snapshots[2].image.size_bytes(),
                            [](std::uint8_t value)
                            {
                                return value == 0;
                            }),
            "A flat frame has a black response and no corners, even after previous detections and a size change");
}

void test_feature_matching()
{
    visionlab::Frame frame{29, 1.25, visionlab::Image(480, 320)};
    cv::Mat texture(320, 480, CV_8UC3, frame.image.data(), frame.image.stride_bytes());
    texture.setTo(cv::Scalar(25, 25, 25));
    cv::RNG random(12345);
    for (int index = 0; index < 180; ++index)
    {
        const cv::Point center(random.uniform(30, 450), random.uniform(30, 290));
        const int value = random.uniform(70, 255);
        cv::circle(texture, center, random.uniform(2, 12), cv::Scalar(value, value, value), index % 3 == 0 ? 2 : -1);
    }
    cv::putText(texture, "VISION 27", cv::Point(75, 170), cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(255, 255, 255), 2);
    for (const visionlab::FeatureMethod method : {visionlab::FeatureMethod::Sift, visionlab::FeatureMethod::Orb})
    {
        const auto report = std::make_shared<visionlab::FeatureMatchReport>();
        visionlab::Pipeline pipeline;
        visionlab::add_feature_matching(pipeline, method, report);
        const visionlab::FrameResult result = pipeline.process(frame, true);
        require(result.snapshots.size() == 5 && result.frame.index == 29 && result.frame.timestamp_seconds == frame.timestamp_seconds,
                "Matching preserves frame identity and exposes four stages");
        require(report->source_keypoints > 20 && report->target_keypoints > 20 && report->ratio_matches >= 10 && report->inliers >= 8 &&
                    report->inliers <= report->ratio_matches,
                "Textured transformed images produce keypoints, matches and geometric inliers");
        require(report->median_known_transform_error && *report->median_known_transform_error < 3.0,
                "Matches agree with independent known rotation and scale");
        require(result.frame.image.width() == 960 && result.frame.image.height() == 374 && result.boxes.empty(),
                "Matching canvas dimensions and overlay coordinates");
        const visionlab::Image& original = result.snapshots.front().image;
        require(std::equal(original.data(), original.data() + original.size_bytes(), frame.image.data()),
                "Feature drawing preserves captured source");
        const visionlab::FrameResult headless = pipeline.process(frame);
        require(headless.snapshots.empty() && report->inliers >= 8 && report->median_known_transform_error &&
                    *report->median_known_transform_error < 3.0,
                "Repeated headless matching needs no snapshots or previous frame state");
        texture.setTo(cv::Scalar(25, 25, 25));
        pipeline.process(frame);
        require(report->source_keypoints == 0 && report->ratio_matches == 0 && report->inliers == 0 &&
                    !report->median_known_transform_error,
                "Blank input clears previous matches and accuracy report");
        pipeline.process(visionlab::Frame{0, std::nullopt, visionlab::Image(1, 1)});
        require(report->inliers == 0, "Tiny inputs do not fail inside feature pyramids");
        // Restore the fixture for the next algorithm.
        std::copy(original.data(), original.data() + original.size_bytes(), frame.image.data());
    }
}

void test_proto_tracker_pipeline()
{
    const visionlab::app::PipelineCatalog catalog;
    visionlab::Pipeline pipeline = catalog.create("practice-28-proto-tracker");
    for (std::uint64_t index = 0; index < 5; ++index)
    {
        visionlab::Frame frame{index, index / 30.0, visionlab::Image(160, 100)};
        cv::Mat image(100, 160, CV_8UC3, frame.image.data(), frame.image.stride_bytes());
        image.setTo(cv::Scalar(25, 25, 25));
        cv::circle(image, cv::Point(40 + static_cast<int>(index) * 3, 50), 17, cv::Scalar(0, 140, 255), -1);
        const visionlab::FrameResult result = pipeline.process(frame, index == 0);
        require(result.boxes.size() == 1 && result.boxes[0].label == (index == 0 ? "ID 1 (new)" : "ID 1 (matched)"),
                "Detector and tracker retain a moving circle's ID with or without snapshots");
        if (index == 0)
        {
            require(result.snapshots.size() == 7 && result.snapshots[5].boxes[0].label.find("orange round object") == 0 &&
                        result.snapshots[6].boxes[0].label == "ID 1 (new)",
                    "Detection snapshot remains separate from tracked IDs");
        }
    }
}

} // namespace

int main()
{
    // Fixture lives in the CTest build directory and never enters the repository.
    const std::filesystem::path fixture = std::filesystem::current_path() / "visionlab-test.avi";
    const std::filesystem::path still_fixture = std::filesystem::current_path() / "visionlab-test.png";
    try
    {
        test_grayscale();
        test_mean_blur();
        test_binary_threshold();
        test_morphology();
        test_hsv_color();
        test_adaptive_threshold();
        test_otsu_threshold();
        test_morphology_ex_equivalence();
        test_kernel_shapes();
        test_gradient_direction();
        test_sobel_smoothing();
        test_canny_walkthrough();
        test_hsv_cleanup();
        test_connected_components();
        test_component_area_filter();
        test_external_contours();
        test_contour_measurements();
        test_contour_hierarchy();
        test_rotated_rectangle();
        test_moments_centroid();
        test_classical_detector();
        test_harris_pipeline();
        test_feature_matching();
        test_proto_tracker_pipeline();
        cv::VideoWriter writer(fixture.string(), cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 25.0, cv::Size(32, 24));
        require(writer.isOpened(), "MJPEG writer unavailable; cannot generate video test fixture");
        writer.write(cv::Mat(24, 32, CV_8UC3, cv::Scalar(0, 0, 255)));
        writer.write(cv::Mat(24, 32, CV_8UC3, cv::Scalar(255, 0, 0)));
        writer.release();
        std::unique_ptr<visionlab::FrameSource> source = visionlab::make_video_source(fixture.string());
        const std::optional<visionlab::Frame> first = source->next();
        const std::optional<visionlab::Frame> second = source->next();
        require(first && second && !source->next(), "Video frame count and EOF");
        require(first->index == 0 && second->index == 1, "Video indices");
        require(first->image.width() == 32 && first->image.height() == 24, "Video dimensions");
        require(first->image.data()[2] > 240 && first->image.data()[0] < 15 && second->image.data()[0] > 240,
                "BGR order and independent decoded storage");
        require(second->timestamp_seconds && std::abs(*second->timestamp_seconds - 0.04) < 1e-6, "Nominal media time");
        source.reset();
        {
            const visionlab::app::PipelineCatalog video_pipelines;
            visionlab::app::ViewerSession video_session(video_pipelines, 10);
            require(video_session.select_source(visionlab::app::SourceCatalog::from_path(fixture)), "Open video session");
            require(video_session.next_frame() && !video_session.next_frame() && video_session.ended(), "Detect actual video EOF");
            require(video_session.previous_frame() && !video_session.ended() && video_session.snapshot()->image.data()[2] > 240,
                    "Previous frame restores the red frame after EOF");
            require(video_session.next_frame() && video_session.ended() && video_session.snapshot()->image.data()[0] > 240,
                    "Forward history restores the blue frame without another read at EOF");
            require(video_session.previous_frame() && video_session.select_pipeline("pass-through") && video_session.next_frame() &&
                        video_session.result()->frame.index == 1 && video_session.snapshot()->image.data()[0] > 240,
                    "Pipeline change on an earlier video frame resumes from its successor");
        }
        std::filesystem::remove(fixture);
        bool rejected = false;
        try
        {
            visionlab::make_video_source(fixture.string());
        }
        catch (const std::runtime_error&)
        {
            rejected = true;
        }
        require(rejected, "Missing video must fail clearly");
        require(cv::imwrite(still_fixture.string(), cv::Mat(13, 17, CV_8UC3, cv::Scalar(11, 22, 33))), "Write image fixture");
        std::unique_ptr<visionlab::FrameSource> still = visionlab::make_image_source(still_fixture);
        const std::optional<visionlab::Frame> image = still->next();
        require(image && image->image.width() == 17 && image->image.height() == 13 && image->image.data()[0] == 11 &&
                    image->image.data()[2] == 33 && !still->next() && !still->next(),
                "Image decode, BGR layout, and one-frame EOF");
        visionlab::app::PipelineCatalog pipelines;
        visionlab::app::ViewerSession session(pipelines, 10);
        require(session.select_source(visionlab::app::SourceCatalog::from_path(still_fixture)), "Switch session to image");
        require(session.ended() && !session.playing() && session.snapshot()->image.width() == 17,
                "Still images stay visible with playback disabled");
        require(session.restart() && session.snapshot()->image.data()[1] == 22, "Restart image source");
        require(!session.can_previous_frame() && !session.previous_frame(), "Still images have no previous frame");
        std::filesystem::remove(still_fixture);
        std::cout << "OpenCV video contracts passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        std::error_code ignored;
        std::filesystem::remove(fixture, ignored);
        std::filesystem::remove(still_fixture, ignored);
        return 1;
    }
}

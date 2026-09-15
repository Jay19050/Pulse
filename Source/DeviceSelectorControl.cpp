#include "DeviceSelectorControl.h"
#include "PulseTheme.h"
#include <cmath>

namespace
{
    // The popup list shown inside the CallOutBox when the pill is clicked.
    // File-local: nothing outside this .cpp ever needs to name it directly.
    class DeviceListBox final : public juce::Component
    {
    public:
        DeviceListBox(std::vector<AudioEngine::DeviceInfo> devicesIn,
                      juce::String currentIdIn,
                      std::function<void(juce::String)> onPick)
            : items(std::move(devicesIn)), currentId(std::move(currentIdIn)), callback(std::move(onPick))
        {
            constexpr int rowHeight = 32;
            const int count = juce::jmax(1, (int) items.size());
            setSize(260, rowHeight * count);
        }

        void paint(juce::Graphics& g) override
        {
            g.fillAll(juce::Colour(PulseTheme::SurfaceRaised));

            if (items.empty())
            {
                g.setColour(juce::Colour(PulseTheme::MutedText));
                g.setFont(12.0f);
                g.drawText("No output devices found", getLocalBounds(), juce::Justification::centred);
                return;
            }

            const int rowHeight = getHeight() / (int) items.size();

            for (size_t i = 0; i < items.size(); ++i)
            {
                const auto row = juce::Rectangle<int>(0, (int) i * rowHeight, getWidth(), rowHeight).toFloat();
                const bool isHovered = (int) i == hoverIndex;
                const bool isSelected = items[i].id == currentId
                                         || (currentId.isEmpty() && items[i].isDefault);

                if (isHovered)
                {
                    g.setColour(juce::Colour(PulseTheme::WidgetBackground));
                    g.fillRect(row);
                }

                auto textArea = row.reduced(12.0f, 0.0f);
                if (isSelected)
                    textArea.removeFromRight(16.0f);

                g.setColour(isSelected ? juce::Colour(PulseTheme::Accent)
                                       : juce::Colour(PulseTheme::DefaultText).withAlpha(isHovered ? 1.0f : 0.85f));
                g.setFont(12.5f);

                juce::String label = items[i].name;
                if (items[i].isDefault)
                    label += "  (Default)";

                g.drawText(label, textArea, juce::Justification::centredLeft);

                if (isSelected)
                {
                    g.setColour(juce::Colour(PulseTheme::Accent));
                    g.fillEllipse(row.getRight() - 20.0f, row.getCentreY() - 3.0f, 6.0f, 6.0f);
                }
            }
        }

        void mouseMove(const juce::MouseEvent& e) override
        {
            const int next = rowAt(e.y);
            if (next != hoverIndex)
            {
                hoverIndex = next;
                repaint();
            }
        }

        void mouseDown(const juce::MouseEvent& e) override
        {
            const int idx = rowAt(e.y);
            if (idx >= 0 && idx < (int) items.size() && callback != nullptr)
                callback(items[(size_t) idx].id);

            if (auto* box = findParentComponentOfClass<juce::CallOutBox>())
                box->dismiss();
        }

    private:
        int rowAt(int y) const
        {
            if (items.empty())
                return -1;
            const int rowHeight = getHeight() / (int) items.size();
            return rowHeight > 0 ? y / rowHeight : -1;
        }

        std::vector<AudioEngine::DeviceInfo> items;
        juce::String currentId;
        std::function<void(juce::String)> callback;
        int hoverIndex = -1;
    };
}

DeviceSelectorControl::DeviceSelectorControl()
{
    setOpaque(false);
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void DeviceSelectorControl::setDevices(std::vector<AudioEngine::DeviceInfo> newDevices, const juce::String& newSelectedId)
{
    devices = std::move(newDevices);
    selectedId = newSelectedId;
    repaint();
}

void DeviceSelectorControl::setSelectedId(const juce::String& id)
{
    selectedId = id;
    repaint();
}

juce::String DeviceSelectorControl::currentDisplayName() const
{
    for (const auto& d : devices)
    {
        if (d.id == selectedId || (selectedId.isEmpty() && d.isDefault))
            return d.name;
    }
    return devices.empty() ? "No output devices" : "Select a device";
}

void DeviceSelectorControl::resized()
{
    auto area = getLocalBounds().toFloat();
    pillBounds = area.removeFromTop(34.0f);
    area.removeFromTop(8.0f);
    refreshBounds = area.removeFromTop(20.0f);
}

void DeviceSelectorControl::mouseMove(const juce::MouseEvent& e)
{
    const bool overPill = pillBounds.contains(e.position);
    const bool overRefresh = refreshBounds.contains(e.position);

    if (overPill != pillHover || overRefresh != refreshHover)
    {
        pillHover = overPill;
        refreshHover = overRefresh;
        repaint();
    }
}

void DeviceSelectorControl::mouseExit(const juce::MouseEvent&)
{
    if (pillHover || refreshHover)
    {
        pillHover = false;
        refreshHover = false;
        repaint();
    }
}

void DeviceSelectorControl::mouseDown(const juce::MouseEvent& e)
{
    if (pillBounds.contains(e.position))
        showDeviceMenu();
    else if (refreshBounds.contains(e.position) && onRefreshRequested != nullptr)
        onRefreshRequested();
}

void DeviceSelectorControl::showDeviceMenu()
{
    auto content = std::make_unique<DeviceListBox>(devices, selectedId,
        [this](juce::String id)
        {
            if (onDeviceSelected != nullptr)
                onDeviceSelected(id);
        });

    const auto screenArea = localAreaToGlobal(pillBounds.toNearestInt());
    juce::CallOutBox::launchAsynchronously(std::move(content), screenArea, getTopLevelComponent());
}

void DeviceSelectorControl::paint(juce::Graphics& g)
{
    PulseTheme::panelBackground(g, pillBounds, 6.0f,
                                 pillHover ? PulseTheme::SurfaceRaised : PulseTheme::WidgetBackground);

    auto pillContent = pillBounds.reduced(12.0f, 0.0f);

    // Chevron on the right.
    auto chevronArea = pillContent.removeFromRight(16.0f);
    juce::Path chevron;
    const float cx = chevronArea.getCentreX(), cy = chevronArea.getCentreY();
    chevron.startNewSubPath(cx - 4.0f, cy - 2.5f);
    chevron.lineTo(cx, cy + 2.5f);
    chevron.lineTo(cx + 4.0f, cy - 2.5f);
    g.setColour(juce::Colour(PulseTheme::MutedText));
    g.strokePath(chevron, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    g.setColour(juce::Colour(PulseTheme::DefaultText));
    g.setFont(12.5f);
    g.drawText(currentDisplayName(), pillContent, juce::Justification::centredLeft, true);

    // Refresh affordance: small circular-arrow glyph + label.
    auto refreshContent = refreshBounds;
    auto iconArea = refreshContent.removeFromLeft(16.0f).withSizeKeepingCentre(12.0f, 12.0f);

    const juce::Colour refreshColour = juce::Colour(PulseTheme::MutedText).withAlpha(refreshHover ? 1.0f : 0.75f);
    g.setColour(refreshColour);

    juce::Path arrow;
    arrow.addArc(iconArea.getX(), iconArea.getY(), iconArea.getWidth(), iconArea.getHeight(),
                 juce::MathConstants<float>::pi * 0.2f, juce::MathConstants<float>::twoPi * 0.85f, true);
    g.strokePath(arrow, juce::PathStrokeType(1.4f));
    // Arrowhead
    const float ax = iconArea.getCentreX() + iconArea.getWidth() * 0.5f * std::cos(juce::MathConstants<float>::pi * 0.2f);
    const float ay = iconArea.getCentreY() + iconArea.getHeight() * 0.5f * std::sin(juce::MathConstants<float>::pi * 0.2f);
    g.fillEllipse(ax - 1.5f, ay - 1.5f, 3.0f, 3.0f);

    refreshContent.removeFromLeft(6.0f);
    g.setFont(11.5f);
    g.drawText("Refresh Devices", refreshContent, juce::Justification::centredLeft);
}

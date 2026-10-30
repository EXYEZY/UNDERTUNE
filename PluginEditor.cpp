#include "PluginEditor.h"
#include <BinaryData.h>

namespace
{
    const juce::Colour bg (0xff100812), panel (0xff1b101e), pink (0xffff3da8), pale (0xffffc4e8), muted (0xffb99ab6);
}

UndertuneAudioProcessorEditor::UndertuneAudioProcessorEditor (UndertuneAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p),
      marinPanel (juce::ImageCache::getFromMemory (BinaryData::undertune_marin_panel_png, BinaryData::undertune_marin_panel_pngSize))
{
    setSize (1000, 650);
    title.setText ("UNDERTUNE  ♥", juce::dontSendNotification);
    title.setFont (juce::Font (juce::FontOptions (38.0f, juce::Font::bold)));
    title.setColour (juce::Label::textColourId, pink);
    addAndMakeVisible (title);
    subtitle.setText ("V O C A L   P I T C H   C O R R E C T I O N", juce::dontSendNotification);
    subtitle.setFont (juce::Font (juce::FontOptions (11.0f)));
    subtitle.setColour (juce::Label::textColourId, pale);
    addAndMakeVisible (subtitle);

    auto setupLabel = [this] (juce::Label& label, const juce::String& text)
    {
        label.setText (text, juce::dontSendNotification);
        label.setFont (juce::Font (juce::FontOptions (13.0f, juce::Font::bold)));
        label.setColour (juce::Label::textColourId, pale);
        addAndMakeVisible (label);
    };
    setupLabel (modeLabel, "CORRECTION MODE");
    setupLabel (keyLabel, "TONALIDAD / KEY");
    setupLabel (scaleLabel, "ESCALA / SCALE");

    modeBox.addItem ("FEID  —  NATURAL  ·  RETUNE 20 ms", 1);
    modeBox.addItem ("under  —  FULL ROBOTICO  ·  RETUNE 0 ms", 2);
    keyBox.addItemList ({ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 1);
    scaleBox.addItem ("Major", 1);
    scaleBox.addItem ("Minor", 2);
    for (auto* box : { &modeBox, &keyBox, &scaleBox })
    {
        box->setColour (juce::ComboBox::backgroundColourId, panel);
        box->setColour (juce::ComboBox::textColourId, juce::Colours::white);
        box->setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff7e315f));
        box->setColour (juce::ComboBox::arrowColourId, pink);
        addAndMakeVisible (*box);
    }
    modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (processor.parameters, "MODE", modeBox);
    keyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (processor.parameters, "KEY", keyBox);
    scaleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (processor.parameters, "SCALE", scaleBox);

    bypass.setButtonText ("BYPASS");
    bypass.setColour (juce::ToggleButton::textColourId, pale);
    bypass.setColour (juce::ToggleButton::tickColourId, pink);
    bypass.setColour (juce::ToggleButton::tickDisabledColourId, muted);
    addAndMakeVisible (bypass);
    bypass.onClick = [this] { processor.parameters.getParameter ("BYPASS")->setValueNotifyingHost (bypass.getToggleState() ? 1.0f : 0.0f); };

    footer.setText ("FEID: smoother tuning     ♥     under: hard robotic snap", juce::dontSendNotification);
    footer.setJustificationType (juce::Justification::centredLeft);
    footer.setColour (juce::Label::textColourId, muted);
    footer.setFont (juce::Font (juce::FontOptions (11.0f)));
    addAndMakeVisible (footer);
}

void UndertuneAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (bg);
    g.setColour (juce::Colour (0xff2a1028));
    g.fillRoundedRectangle (18.0f, 18.0f, 964.0f, 614.0f, 16.0f);
    g.setColour (juce::Colour (0xff110a15));
    g.fillRoundedRectangle (24.0f, 24.0f, 952.0f, 602.0f, 13.0f);

    if (marinPanel.isValid())
    {
        auto area = juce::Rectangle<int> (590, 28, 376, 584);
        g.saveState();
        g.reduceClipRegion (area);
        g.setOpacity (0.96f);
        g.drawImageWithin (marinPanel, area.getX(), area.getY(), area.getWidth(), area.getHeight(), juce::RectanglePlacement::centred); 
        g.restoreState();
        g.setColour (juce::Colour (0x99100812));
        g.fillRect (590, 28, 24, 584);
    }

    g.setColour (pink);
    g.fillRoundedRectangle (36.0f, 108.0f, 520.0f, 3.0f, 1.5f);
    g.setColour (juce::Colour (0xff4b2047));
    g.drawRoundedRectangle (34.0f, 150.0f, 526.0f, 132.0f, 12.0f, 1.2f);
    g.setColour (juce::Colour (0x33ff3da8));
    g.fillRoundedRectangle (40.0f, 156.0f, 514.0f, 120.0f, 10.0f);
    g.setColour (juce::Colour (0xff4b2047));
    g.drawRoundedRectangle (34.0f, 318.0f, 526.0f, 220.0f, 12.0f, 1.2f);
    g.setColour (pink.withAlpha (0.75f));
    g.drawLine (40.0f, 578.0f, 555.0f, 578.0f, 1.0f);
    g.setColour (pale);
    g.setFont (juce::Font (juce::FontOptions (13.0f, juce::Font::bold)));
    g.drawText ("FEID  ♥  NATURAL", 48, 160, 490, 22, juce::Justification::centredLeft);
    g.drawText ("under  ♥  FULL ROBOTICO", 48, 204, 490, 22, juce::Justification::centredLeft);
    g.setColour (muted);
    g.setFont (juce::Font (juce::FontOptions (10.0f)));
    g.drawText ("JAPANESE STREET FASHION  /  GOTHIC COSPLAY INSPIRED UI", 40, 594, 530, 18, juce::Justification::centredLeft);
}

void UndertuneAudioProcessorEditor::resized()
{
    title.setBounds (38, 28, 510, 58);
    subtitle.setBounds (42, 80, 510, 20);
    modeLabel.setBounds (50, 164, 490, 18);
    modeBox.setBounds (50, 228, 490, 32);
    keyLabel.setBounds (50, 334, 220, 20);
    keyBox.setBounds (50, 361, 220, 36);
    scaleLabel.setBounds (302, 334, 220, 20);
    scaleBox.setBounds (302, 361, 220, 36);
    bypass.setBounds (50, 432, 120, 28);
    footer.setBounds (50, 480, 490, 28);
}

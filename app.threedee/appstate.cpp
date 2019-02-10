#include "appstate.h"

#include <glad/glad.h>
#include <iostream>
#include <memory>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

const float noteLabelWidth = 50.0f;
const float stepWidth = 20.0f;

AppState::AppState(class Mixer *mixer, class Stepper *stepper, class IBankManager *banks)
    : _mixer(mixer), _stepper(stepper), _banks(banks), _showLibrary(false), _showEditor(false), _showInspector(true),
      _showMixer(true), _showSmartControls(true), _showQuickHelp(false),
      _showSystemEffectsEditor(true), _showInsertEffectsEditor(true), _showTrackEffectsEditor(true),
      _showADNoteEditor(true), _showSUBNoteEditor(true), _showPADNoteEditor(true),
      _showTrackTypeChanger(-1),
      _currentInsertEffect(-1), _currentSystemEffect(-1), _currentTrackEffect(-1), _currentBank(0),
      _activeTrack(0), _activeTrackInstrument(0), _activePattern(-1),
      _sequencerVerticalZoom(40), _sequencerHorizontalZoom(40)
{}

AppState::~AppState() = default;

TrackRegion::TrackRegion()
    : previewImage(0), repeat(0)
{}

TrackRegion::~TrackRegion()
{
    CleanupPreviewImage();
}

void TrackRegion::CleanupPreviewImage()
{
    if (previewImage != 0)
    {
        glDeleteTextures(1, &previewImage);
    }
}
struct colour
{
    unsigned char r, g, b;
};

void setPixel(colour *buffer, int x, int y, int w, int h)
{
    if (x < 0) x = 0;
    if (x >= w) x = w - 1;
    if (y < 0) y = 0;
    if (y >= h) y = h - 1;

    buffer[(y * w) + x].r = 155;
    buffer[(y * w) + x].g = 155;
    buffer[(y * w) + x].b = 155;
}

void TrackRegion::UpdatePreviewImage()
{
    int minNote = 999;
    int maxNote = -999;
    for (int i = 0; i < NUM_MIDI_NOTES; i++)
    {
        if (!eventsByNote[i].empty())
        {
            if (minNote > i) minNote = i;
            if (maxNote < i) maxNote = i;
        }
    }

    minNote -= 1;
    maxNote += 1;

    if (maxNote < minNote)
    {
        CleanupPreviewImage();
        return;
    }

    if (previewImage == 0)
    {
        glGenTextures(1, &previewImage);
    }

    auto width = startAndEnd[1] - startAndEnd[0];
    auto height = maxNote - minNote;

    auto imageHeight = 8;
    while (imageHeight < height)
    {
        imageHeight += 8;
    }

    size_t size = size_t(128 * imageHeight);
    auto buffer = std::unique_ptr<colour>(new colour[size]);
    memset(buffer.get(), 255, size * sizeof(colour));

    for (int i = 0; i <= height; i++)
    {
        if ((i + minNote) >= NUM_MIDI_NOTES)
        {
            continue;
        }
        if (eventsByNote[i + minNote].empty())
        {
            continue;
        }

        int y = std::abs((imageHeight - 1) - int(float(i) * (float(imageHeight - 1) / float(height))));
        for (auto &n : eventsByNote[i + minNote])
        {
            int xFrom = int(float(n.values[0] / width) * 128.0f);
            int xTo = int(float(n.values[1] / width) * 128.0f);
            for (int j = xFrom; j < xTo; j++)
            {
                setPixel(buffer.get(), j, y, 128, imageHeight);
            }
        }
    }

    glBindTexture(GL_TEXTURE_2D, previewImage);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer.get());

    stbi_write_png("c:\\temp\\previewImage.png", 128, imageHeight, 3, buffer.get(), 0);
}

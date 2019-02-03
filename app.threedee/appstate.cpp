#include "appstate.h"

#include <glad/glad.h>
#include <iostream>

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
    : previewImage(0)
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

void setPixel(unsigned char *buffer, int x, int y, int w, int h)
{
    if (x < 0) x = 0;
    if (x >= w) x = w - 1;
    if (y < 0) y = 0;
    if (y >= h) y = h - 1;

    buffer[(y * (w * 3)) + (x * 3)] = 0;
    buffer[(y * (w * 3)) + (x * 3) + 1] = 0;
    buffer[(y * (w * 3)) + (x * 3) + 2] = 255;
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
    auto height = maxNote - minNote + 1;

    size_t size = size_t(128 * 128 * 3);
    auto buffer = new unsigned char[size];
    memset(buffer, 255, size);

    for (int i = 0; i <= height; i++)
    {
        if (eventsByNote[i + minNote].empty()) continue;

        int y = std::abs(124 - int(i * (124 / height)));
        for (auto &n : eventsByNote[i + minNote])
        {
            int xFrom = (n.values[0] / width) * 124;
            std::cout << "xFrom = (" << n.values[0] << " / " << width << ") * 124" << std::endl;
            int xTo = (n.values[1] / width) * 124;
            std::cout << "xTo = (" << n.values[1] << " / " << width << ") * 124" << std::endl;
            std::cout << xFrom << "->" << xTo << std::endl;
            for (int j = xFrom; j < xTo; j++)
            {
                setPixel(buffer, j, y, 128, 128);
            }
        }
    }

    glBindTexture(GL_TEXTURE_2D, previewImage);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);

    stbi_write_png("c:\\temp\\previewImage.png", 128, 128, 3, buffer, 0);
    delete[] buffer;
}

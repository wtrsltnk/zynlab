#include "appstate.h"

const float noteLabelWidth = 50.0f;
const float stepWidth = 20.0f;

AppState::AppState(class Mixer *mixer)
    : _mixer(mixer), _showLibrary(false), _showEditor(false), _showInspector(false), _showMixer(true),
      _showSystemEffectsEditor(false), _showInsertEffectsEditor(false), _showChannelEffectsEditor(false),
      _showADNoteEditor(true), _showSUBNoteEditor(true), _showPADNoteEditor(true),
      _showChannelInstrumentSelector(-1), _showChannelTypeChanger(-1),
      _currentInsertEffect(-1), _currentSystemEffect(-1), _currentChannelEffect(-1), _currentBank(0),
      _activeChannel(0), _activeChannelInstrument(0), _activePattern(-1)
{}

AppState::~AppState() = default;

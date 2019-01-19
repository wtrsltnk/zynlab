#include "appstate.h"

const float noteLabelWidth = 50.0f;
const float stepWidth = 20.0f;

AppState::AppState(class Mixer *mixer, class Stepper *stepper)
    : _mixer(mixer), _stepper(stepper), _showLibrary(false), _showEditor(false), _showInspector(false), _showMixer(true), _showSmartControls(true),
      _showSystemEffectsEditor(true), _showInsertEffectsEditor(true), _showChannelEffectsEditor(true),
      _showADNoteEditor(true), _showSUBNoteEditor(true), _showPADNoteEditor(true),
      _showChannelTypeChanger(-1),
      _currentInsertEffect(-1), _currentSystemEffect(-1), _currentChannelEffect(-1), _currentBank(0),
      _activeChannel(0), _activeChannelInstrument(0), _activePattern(-1),
      _sequencerChannelHeight(40)
{}

AppState::~AppState() = default;

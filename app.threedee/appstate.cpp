#include "appstate.h"

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

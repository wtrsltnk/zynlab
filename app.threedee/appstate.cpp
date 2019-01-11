#include "appstate.h"

AppState::AppState(class Mixer *mixer)
    : _mixer(mixer), _showLibrary(false), _showEditor(false), _showInspector(false), _showMixer(true),
      _showInstrumentEditor(false), _showSystemEffectsEditor(false), _showInsertEffectsEditor(false), _showInstrumentEffectsEditor(false),
      _showADNoteEditor(true), _showSUBNoteEditor(true), _showPADNoteEditor(true),
      _openSelectInstrument(-1), _openChangeInstrumentType(-1),
      _currentInsertEffect(-1), _currentSystemEffect(-1), _currentInstrumentEffect(-1), _currentBank(0)
{}

AppState::~AppState() = default;

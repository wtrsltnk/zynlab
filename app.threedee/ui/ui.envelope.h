#ifndef UI_ENVELOPE_H
#define UI_ENVELOPE_H

#include <zyn.synth/EnvelopeParams.h>

namespace zyn {
namespace ui {

class Envelope
{
private:
    char const *_label;

public:
    Envelope(char const *label);

    void Render(EnvelopeParams *params);
};

} // namespace ui
} // namespace zyn

#endif // UI_ENVELOPE_H

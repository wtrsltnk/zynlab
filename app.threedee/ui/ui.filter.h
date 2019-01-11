#ifndef UI_FILTER_H
#define UI_FILTER_H

#include <zyn.dsp/FilterParams.h>

namespace zyn {
namespace ui {

class Filter
{
public:
    Filter();

    void Render(FilterParams *params);
};

} // namespace ui
} // namespace zyn

#endif // UI_FILTER_H

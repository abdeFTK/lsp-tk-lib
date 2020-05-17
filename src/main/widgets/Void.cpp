/*
 * Void.cpp
 *
 *  Created on: 11 апр. 2020 г.
 *      Author: sadko
 */

#include <lsp-plug.in/tk/tk.h>
#include <lsp-plug.in/common/debug.h>

namespace lsp
{
    namespace tk
    {
        const w_class_t Void::metadata      = { "Void", &Widget::metadata };
        
        Void::Void(Display *dpy):
            Widget(dpy),
            sConstraints(&sProperties)
        {
            pClass          = &metadata;
        }
        
        Void::~Void()
        {
        }

        status_t Void::init()
        {
            status_t res = Widget::init();
            if (res != STATUS_OK)
                return res;

            sConstraints.bind("size.constraints", &sStyle);

            Style *sclass = style_class();
            if (sclass != NULL)
                sConstraints.init(sclass, -1, -1, -1, -1);

            return res;
        }

        void Void::render(ws::ISurface *s, bool force)
        {
            if ((sSize.nWidth > 0) && (sSize.nHeight > 0))
            {
                lsp::Color bgColor(sBgColor);
                s->fill_rect(sSize.nLeft, sSize.nTop, sSize.nWidth, sSize.nHeight, bgColor);
            }
        }

        void Void::size_request(ws::size_limit_t *r)
        {
            // Add external size constraints
            float scaling = lsp_max(0.0f, sScaling.get());
            sConstraints.compute(r, scaling);
        }

        void Void::realize(const ws::rectangle_t *r)
        {
            lsp_trace("x=%d, y=%d, w=%d, h=%d", int(r->nLeft), int(r->nTop), int(r->nWidth), int(r->nHeight));
            Widget::realize(r);
        }

    } /* namespace tk */
} /* namespace lsp */

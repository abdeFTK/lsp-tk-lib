/*
 * Copyright (C) 2020 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2020 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-tk-lib
 * Created on: 8 авг. 2020 г.
 *
 * lsp-tk-lib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * lsp-tk-lib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with lsp-tk-lib. If not, see <https://www.gnu.org/licenses/>.
 */

#include <lsp-plug.in/tk/tk.h>
#include <lsp-plug.in/stdlib/math.h>
#include <private/tk/style/BuiltinStyle.h>

namespace lsp
{
    namespace tk
    {
        STYLE_INITIALIZER_BEGIN(ComboBox, WidgetContainer);

            prop::Integer::init("border.size", style, 1);
            prop::Integer::init("border.gap", style, 1);
            prop::Integer::init("border.radius", style, 4);
            prop::Integer::init("spin.size", style, 10);
            prop::Integer::init("spin.separator", style, 1);
            prop::Color::init("color", style, "#ffffff");
            prop::Color::init("spin.color", style, "#ffffff");
            prop::Color::init("text.color", style, "#000000");
            prop::Color::init("spin.text.color", style, "#000000");
            prop::Color::init("border.color", style, "#000000");
            prop::Color::init("border.gap.color", style, "#cccccc");
            prop::Boolean::init("opened", style, false);
            prop::TextFitness::init("text.fit", style);
            prop::Font::init("font", style);
            prop::SizeConstraints::init("size.constraints", style, -1, -1, -1, 0);
            prop::TextLayout::init("text.layout", style, -1.0f, 0.0f);
            prop::String::init("text.empty", style);

        STYLE_INITIALIZER_END(ComboBox, "ComboBox");
        LSP_BUILTIN_STYLE(ComboBox);

        //-----------------------------------------------------------------------------
        // ComboBox popup window implementation
        const w_class_t ComboBox::Window::metadata      = { "ComboBox::Window", &PopupWindow::metadata };

        ComboBox::Window::Window(Display *dpy, ComboBox *cbox):
            PopupWindow(dpy)
        {
            pCBox           = cbox;
            pClass          = &metadata;
        }

        status_t ComboBox::Window::on_hide()
        {
            pCBox->sOpened.set(false);
            return STATUS_OK;
        }

        status_t ComboBox::Window::on_show()
        {
            pCBox->sOpened.set(true);
            return STATUS_OK;
        }

        //-----------------------------------------------------------------------------
        // ComboBox list implementation
        const w_class_t ComboBox::List::metadata        = { "ComboBox::List", &ListBox::metadata };

        ComboBox::List::List(Display *dpy, ComboBox *cbox):
            ListBox(dpy)
        {
            pCBox           = cbox;
            pClass          = &metadata;
        }

        void ComboBox::List::property_changed(Property *prop)
        {
            ListBox::property_changed(prop);
            if (vItems.is(prop))
                pCBox->query_resize();
        }

        status_t ComboBox::List::on_submit()
        {
            pCBox->sOpened.set(false);
            return pCBox->sSlots.execute(SLOT_SUBMIT, pCBox, NULL);
        }

        status_t ComboBox::List::on_change()
        {
            ListBoxItem *it  = vSelected.any();
            ListBoxItem *old = pCBox->sSelected.set(it);

            if (old != it)
                pCBox->sSlots.execute(SLOT_CHANGE, pCBox, NULL);

            return STATUS_OK;
        }

        //-----------------------------------------------------------------------------
        // ComboBox implementation
        const w_class_t ComboBox::metadata              = { "ComboBox", &WidgetContainer::metadata };

        ComboBox::ComboBox(Display *dpy):
            WidgetContainer(dpy),
            sLBox(dpy, this),
            sWindow(dpy, this),
            sBorderSize(&sProperties),
            sBorderGap(&sProperties),
            sBorderRadius(&sProperties),
            sSpinSize(&sProperties),
            sSpinSeparator(&sProperties),
            sColor(&sProperties),
            sSpinColor(&sProperties),
            sTextColor(&sProperties),
            sSpinTextColor(&sProperties),
            sBorderColor(&sProperties),
            sBorderGapColor(&sProperties),
            sOpened(&sProperties),
            sTextFit(&sProperties),
            sFont(&sProperties),
            sConstraints(&sProperties),
            sTextLayout(&sProperties),
            sEmptyText(&sProperties),
            sSelected(&sProperties)
        {
            sTArea.nLeft    = 0;
            sTArea.nTop     = 0;
            sTArea.nWidth   = 0;
            sTArea.nHeight  = 0;

            sSArea.nLeft    = 0;
            sSArea.nTop     = 0;
            sSArea.nWidth   = 0;
            sSArea.nHeight  = 0;

            sVArea.nLeft    = 0;
            sVArea.nTop     = 0;
            sVArea.nWidth   = 0;
            sVArea.nHeight  = 0;

            nMBState        = 0;

            pClass          = &metadata;
        }

        ComboBox::~ComboBox()
        {
            nFlags     |= FINALIZED;
            do_destroy();
        }

        void ComboBox::destroy()
        {
            nFlags     |= FINALIZED;
            do_destroy();
            WidgetContainer::destroy();
        }

        void ComboBox::do_destroy()
        {
            sLBox.set_parent(NULL);
            sLBox.destroy();
            sWindow.destroy();
        }

        status_t ComboBox::init()
        {
            handler_id_t id;

            // Initialize widgets
            status_t result = WidgetContainer::init();
            if (result == STATUS_OK)
                result  = sWindow.init();
            if (result == STATUS_OK)
                result  = sLBox.init();
            if (result != STATUS_OK)
                return result;

            // Configure Window
            sWindow.add(&sLBox);
            sWindow.add_arrangement(A_BOTTOM, 0, true);
            sWindow.add_arrangement(A_TOP, 0, true);
            sWindow.layout()->set_scale(1.0f);

            sBorderSize.bind("border.size", &sStyle);
            sBorderGap.bind("border.gap", &sStyle);
            sBorderRadius.bind("border.radius", &sStyle);
            sSpinSize.bind("spin.size", &sStyle);
            sSpinSeparator.bind("spin.separator", &sStyle);
            sColor.bind("color", &sStyle);
            sSpinColor.bind("spin.color", &sStyle);
            sTextColor.bind("text.color", &sStyle);
            sSpinTextColor.bind("spin.text.color", &sStyle);
            sBorderColor.bind("border.color", &sStyle);
            sBorderGapColor.bind("border.gap.color", &sStyle);
            sOpened.bind("opened", &sStyle);
            sTextFit.bind("text.fit", &sStyle);
            sFont.bind("font", &sStyle);
            sConstraints.bind("size.constraints", &sStyle);
            sTextLayout.bind("text.layout", &sStyle);
            sEmptyText.bind(&sStyle, pDisplay->dictionary());

//            Style *sclass = style_class();
//            if (sclass != NULL)
//            {
//                sBorderSize.init(sclass, 1);
//                sBorderGap.init(sclass, 1);
//                sBorderRadius.init(sclass, 4);
//                sSpinSize.init(sclass, 10);
//                sSpinSeparator.init(sclass, 1);
//                sColor.init(sclass, "#ffffff");
//                sSpinColor.init(sclass, "#ffffff");
//                sTextColor.init(sclass, "#000000");
//                sSpinTextColor.init(sclass, "#000000");
//                sBorderColor.init(sclass, "#000000");
//                sBorderGapColor.init(sclass, "#cccccc");
//                sOpened.init(sclass, false);
//                sTextFit.init(sclass);
//                sFont.init(sclass);
//                sConstraints.init(sclass, -1, -1, -1, 0);
//                sTextLayout.init(sclass, -1.0f, 0.0f);
//            }

            // Bind slots
            id = sSlots.add(SLOT_CHANGE, slot_on_change, self());
            if (id >= 0) id = sSlots.add(SLOT_SUBMIT, slot_on_change, self());
            if (id < 0)
                return -id;

            return STATUS_OK;
        }

        void ComboBox::property_changed(Property *prop)
        {
            WidgetContainer::property_changed(prop);

            if (sBorderSize.is(prop))
                query_resize();
            if (sBorderGap.is(prop))
                query_resize();
            if (sBorderRadius.is(prop))
                query_resize();
            if (sSpinSize.is(prop))
                query_resize();
            if (sSpinSeparator.is(prop))
                query_resize();
            if (sColor.is(prop))
                query_draw();
            if (sSpinColor.is(prop))
                query_draw();
            if (sTextColor.is(prop))
                query_draw();
            if (sSpinTextColor.is(prop))
                query_draw();
            if (sBorderColor.is(prop))
                query_draw();
            if (sBorderGapColor.is(prop))
                query_draw();
            if (sOpened.is(prop))
            {
                bool visible = sWindow.visibility()->get();
                if (sOpened.get() != visible)
                {
                    if (!visible)
                    {
                        ws::rectangle_t r;
                        this->get_padded_screen_rectangle(&r);
                        sWindow.trigger_area()->set(&r);
                        sWindow.trigger_widget()->set(this);
                        sWindow.show(this);

                        sWindow.grab_events(ws::GRAB_DROPDOWN);
                        sWindow.take_focus();
                        sLBox.take_focus();
                    }
                    else
                        sWindow.hide();
                }
            }
            if (sTextFit.is(prop))
                query_resize();
            if (sFont.is(prop))
                query_resize();
            if (sConstraints.is(prop))
                query_resize();
            if (sTextLayout.is(prop))
                query_draw();
            if (sEmptyText.is(prop))
                query_resize();
            if (sSelected.is(prop))
            {
                ListBoxItem *it = sSelected.get();
                if (sLBox.items()->contains(it))
                {
                    sLBox.selected()->clear();
                    sLBox.selected()->add(it);
                }
                else
                    sSelected.set(NULL);
                query_draw();
            }
        }

        void ComboBox::estimate_parameters(alloc_t *alloc, float scaling)
        {
            alloc->radius       = (sBorderRadius.get() > 0) ? lsp_max(1.0f, sBorderRadius.get() * scaling) : 0;
            alloc->border       = (sBorderSize.get() > 0) ? lsp_max(1.0f, sBorderSize.get() * scaling) : 0;
            alloc->bgap         = ((alloc->border > 0) && (sBorderGap.get() > 0)) ? lsp_max(0.0f, sBorderGap.get()) : 0;
            alloc->bsize        = alloc->border + alloc->bgap;
            alloc->swidth       = (sSpinSize.get() > 0) ? lsp_max(1.0f, sSpinSize.get() * scaling) : 0;
            alloc->sborder      = ((alloc->swidth > 0) && (sSpinSeparator.get() > 0)) ? lsp_max(1.0f, sSpinSeparator.get() * scaling) : 0;
            alloc->sgap         = (alloc->sborder > 0) ? alloc->bgap : 0;

            ssize_t rgap        = alloc->radius - lsp_max(0.0f, truncf(M_SQRT1_2 * (alloc->radius - alloc->bsize)));
            alloc->spad         = lsp_max(alloc->bsize, rgap);
        }

        void ComboBox::size_request(ws::size_limit_t *r)
        {
            alloc_t a;
            ws::rectangle_t ta, sa, va; // text area, spin area, split area

            float scaling       = lsp_max(0.0f, sScaling.get());
            estimate_parameters(&a, scaling);

            // Estimate size of the spin
            sa.nWidth           = (a.swidth > 0) ? a.swidth + a.spad : 0;
            sa.nHeight          = (a.swidth > 0) ? a.swidth + a.spad * 2 : 0;

            va.nWidth           = a.sborder + a.sgap * 2;
            va.nHeight          = sa.nHeight;

            // Estimate sizes of the text area
            ws::font_parameters_t fp;
            ws::text_parameters_t tp;
            LSPString text;

            sFont.get_parameters(pDisplay, scaling, &fp);
            ta.nWidth           = 0;
            ta.nHeight          = fp.Height;

            // Estimate the maximum text width
            sEmptyText.format(&text);
            sFont.get_text_parameters(pDisplay, &tp, scaling, &text);
            ta.nWidth           = lsp_max(ta.nWidth,  tp.Width);
            ta.nHeight          = lsp_max(ta.nHeight, tp.Height);

            WidgetList<ListBoxItem> *wl = sLBox.items();
            for (size_t i=0, n=wl->size(); i<n; ++i)
            {
                ListBoxItem *it = wl->get(i);
                if ((it == NULL) || (!it->visibility()->get()))
                    continue;

                it->text()->format(&text);
                sFont.get_text_parameters(pDisplay, &tp, scaling, &text);
                ta.nWidth           = lsp_max(ta.nWidth,  tp.Width);
                ta.nHeight          = lsp_max(ta.nHeight, tp.Height);
            }
            sTextFit.apply(&ta);

            ta.nWidth          += a.spad;
            ta.nHeight         += a.spad * 2;

            // Set-up size of the
            ssize_t extra       = a.radius * 2;
            r->nMinWidth        = lsp_max(extra, ta.nWidth + sa.nWidth + va.nWidth);
            r->nMinHeight       = lsp_max(extra, lsp_max(ta.nHeight, sa.nHeight));
            r->nMaxWidth        = -1;
            r->nMaxHeight       = -1;

            // Apply size constraints
            sConstraints.apply(r, scaling);
        }

        void ComboBox::realize(const ws::rectangle_t *r)
        {
            alloc_t a;
            float scaling       = lsp_max(0.0f, sScaling.get());
            estimate_parameters(&a, scaling);

            // Realize widget
            Widget::realize(r);

            // Estimate size of spin
            sSArea.nWidth       = (a.swidth > 0) ? a.swidth + a.spad : 0;
            sSArea.nHeight      = r->nHeight;
            sSArea.nLeft        = r->nLeft + r->nWidth - sSArea.nWidth;
            sSArea.nTop         = r->nTop;

            // Estimate size of vsep
            sVArea.nWidth       = a.sgap * 2 + a.sborder;
            sVArea.nHeight      = r->nHeight;
            sVArea.nLeft        = sSArea.nLeft - sVArea.nWidth;
            sVArea.nTop         = r->nTop;

            // Estimate size of the text area
            sTArea.nWidth       = sVArea.nLeft - r->nLeft;
            sTArea.nHeight      = r->nHeight;
            sTArea.nLeft        = r->nLeft;
            sTArea.nTop         = r->nTop;
        }

        void ComboBox::draw(ws::ISurface *s)
        {
            alloc_t a;
            bool aa;
            ws::rectangle_t ta, sa, va; // text area, spin area
            float scaling       = lsp_max(0.0f, sScaling.get());
            float bright        = sBrightness.get();
            estimate_parameters(&a, scaling);

            ta                  = sTArea;
            sa                  = sSArea;
            va                  = sVArea;

            ta.nLeft           -= sSize.nLeft;
            ta.nTop            -= sSize.nTop;
            sa.nLeft           -= sSize.nLeft;
            sa.nTop            -= sSize.nTop;
            va.nLeft           -= sSize.nLeft;
            va.nTop            -= sSize.nTop;

            // Draw background
            lsp::Color c(sBgColor);
            s->clear(c);
            aa                  = s->get_antialiasing();

            // Draw the border
            if (a.border > 0)
            {
                c.copy(sBorderColor);
                c.scale_lightness(bright);
                s->set_antialiasing(true);
                s->fill_round_rect(c, SURFMASK_ALL_CORNER, a.radius, 0, 0, sSize.nWidth, sSize.nHeight);

                a.radius        = lsp_max(0, a.radius - a.border);

                ta.nLeft       += a.border;
                ta.nTop        += a.border;
                ta.nWidth      -= a.border;
                ta.nHeight     -= a.border * 2;

                sa.nTop        += a.border;
                sa.nWidth      -= a.border;
                sa.nHeight     -= a.border * 2;

                va.nTop        += a.border;
                va.nHeight     -= a.border * 2;
            }

            // Draw the text area
            {
                ssize_t radius      = a.radius;
                s->set_antialiasing(true);

                // Draw border gap
                if (a.bgap > 0)
                {
                    c.copy(sBorderGapColor);
                    c.scale_lightness(bright);
                    s->fill_round_rect(c, SURFMASK_L_CORNER, radius, &ta);

                    ta.nLeft       += a.bgap;
                    ta.nTop        += a.bgap;
                    ta.nWidth      -= a.bgap;
                    ta.nHeight     -= a.bgap*2;
                    radius          = lsp_max(0, radius - a.bgap);
                }

                // Draw the prime color
                c.copy(sColor);
                c.scale_lightness(bright);
                s->fill_round_rect(c, SURFMASK_L_CORNER, radius, &ta);

                // Now reset parameters of ta before rendering the text
                ta.nLeft        = a.spad;
                ta.nWidth       = sTArea.nWidth  - a.spad;
                ta.nTop         = a.spad;
                ta.nHeight      = sTArea.nHeight - a.spad * 2;

                // Render the text
                LSPString text;
                ws::font_parameters_t fp;
                ws::text_parameters_t tp;
                float halign    = lsp_limit(sTextLayout.halign() + 1.0f, 0.0f, 2.0f);
                float valign    = lsp_limit(sTextLayout.valign() + 1.0f, 0.0f, 2.0f);

                ListBoxItem *sel = sSelected.get();
                if (sel != NULL)
                {
                    if (!sLBox.items()->contains(sel))
                        sel     = NULL;
                    else if (!sel->visibility()->get())
                        sel     = NULL;
                }

                if (sel != NULL)
                    sel->text()->format(&text);
                else
                    sEmptyText.format(&text);

                sFont.get_parameters(s, scaling, &fp);
                sFont.get_text_parameters(s, &tp, scaling, &text);
                float dy        = (ta.nHeight - fp.Height) * 0.5f;
                float dx        = (ta.nWidth  - tp.Width ) * 0.5f;
                ssize_t y       = ta.nTop   + dy * valign + fp.Ascent;
                ssize_t x       = ta.nLeft  + dx * halign - tp.XBearing;

                s->clip_begin(&ta);
                {
                    c.copy(sTextColor);
                    c.scale_lightness(bright);
                    sFont.draw(s, c, x, y, scaling, &text);
                }
                s->clip_end();
            }

            // Draw the background for spin area
            if (sa.nWidth > 0)
            {
                ssize_t radius  = a.radius;
                s->set_antialiasing(true);

                // Draw border gap
                if (a.bgap > 0)
                {
                    c.copy(sBorderGapColor);
                    c.scale_lightness(bright);
                    s->fill_round_rect(c, SURFMASK_R_CORNER, radius, &sa);

                    sa.nTop        += a.bgap;
                    sa.nWidth      -= a.bgap;
                    sa.nHeight     -= a.bgap*2;
                    radius          = lsp_max(0, radius - a.bgap);
                }

                // Draw the prime color
                c.copy(sSpinColor);
                c.scale_lightness(bright);
                s->fill_round_rect(c, SURFMASK_R_CORNER, radius, &sa);

                // Draw arrows
                c.copy(sSpinTextColor);
                c.scale_lightness(bright);
                s->fill_triangle(
                    sa.nLeft + sa.nWidth/6.0f, sa.nTop + (sa.nHeight*3.0f)/7.0f,
                    sa.nLeft + sa.nWidth*0.5f, sa.nTop + sa.nHeight/7.0f,
                    sa.nLeft + (sa.nWidth*5.0f)/6.0f, sa.nTop + (sa.nHeight*3.0f)/7.0f,
                    c);
                s->fill_triangle(
                    sa.nLeft + sa.nWidth/6.0f, sa.nTop + (sa.nHeight*4.0f)/7.0f,
                    sa.nLeft + (sa.nWidth*5.0f)/6.0f, sa.nTop + (sa.nHeight*4.0f)/7.0f,
                    sa.nLeft + sa.nWidth*0.5f, sa.nTop + (sa.nHeight*6.0f)/7.0f,
                    c);
            }

            // Draw the spin separator
            if (va.nWidth > 0)
            {
                s->set_antialiasing(false);

                // Draw separator gap
                if (a.sgap > 0)
                {
                    c.copy(sBorderGapColor);
                    c.scale_lightness(bright);
                    s->fill_rect(c, &va);

                    va.nLeft       += a.sgap;
                    va.nWidth      -= a.sgap*2;
                }

                if (va.nWidth > 0)
                {
                    c.copy(sBorderColor);
                    c.scale_lightness(bright);
                    s->fill_rect(c, &va);
                }
            }

            // Restore antialiaing
            s->set_antialiasing(aa);
        }

        status_t ComboBox::add(Widget *child)
        {
            return sLBox.add(child);
        }

        status_t ComboBox::remove(Widget *child)
        {
            return sLBox.remove(child);
        }

        status_t ComboBox::remove_all()
        {
            return sLBox.remove_all();
        }

        status_t ComboBox::slot_on_change(Widget *sender, void *ptr, void *data)
        {
            ComboBox *_this = widget_ptrcast<ComboBox>(ptr);
            return (_this != NULL) ? _this->on_change() : STATUS_BAD_ARGUMENTS;
        }

        status_t ComboBox::slot_on_submit(Widget *sender, void *ptr, void *data)
        {
            ComboBox *_this = widget_ptrcast<ComboBox>(ptr);
            return (_this != NULL) ? _this->on_submit() : STATUS_BAD_ARGUMENTS;
        }

        status_t ComboBox::on_mouse_down(const ws::event_t *e)
        {
            nMBState       |= 1 << e->nCode;
            return STATUS_OK;
        }

        status_t ComboBox::on_mouse_up(const ws::event_t *e)
        {
            size_t mask     = 1 << e->nCode;
            size_t prev     = nMBState;
            nMBState       &= (~mask);

            if (prev == mask)
            {
                if (e->nCode == ws::MCB_LEFT)
                    sOpened.toggle();
            }

            return STATUS_OK;
        }

        status_t ComboBox::on_mouse_move(const ws::event_t *e)
        {
            return STATUS_OK;
        }

        bool ComboBox::scroll_item(ssize_t direction, size_t count)
        {
            WidgetList<ListBoxItem> *wl = sLBox.items();
            ListBoxItem *ci  = sSelected.get();
            ListBoxItem *xci = NULL;
            ssize_t curr = (ci != NULL) ? wl->index_of(ci) : -1;
            ssize_t last = wl->size() - 1;

            if (direction < 0)
            {
                while (curr > 0)
                {
                    xci = wl->get(--curr);
                    if ((xci == NULL) || (!xci->visibility()->get()))
                        continue;
                    if ((--count) <= 0)
                        break;
                }
            }
            else
            {
                while (curr < last)
                {
                    xci = wl->get(++curr);
                    if ((xci == NULL) || (!xci->visibility()->get()))
                        continue;
                    if ((--count) <= 0)
                        break;
                }
            }

            if ((xci != NULL) && (xci != ci))
            {
                sSelected.set(xci);
                sSlots.execute(SLOT_CHANGE, this, NULL);
                return true;
            }

            return false;
        }

        status_t ComboBox::on_mouse_scroll(const ws::event_t *e)
        {
            if (e->nCode == ws::MCD_UP)
            {
                if (scroll_item(-1, 1))
                    sSlots.execute(SLOT_SUBMIT, this, NULL);
            }
            else if (e->nCode == ws::MCD_DOWN)
            {
                if (scroll_item(1, 1))
                    sSlots.execute(SLOT_SUBMIT, this, NULL);
            }

            return STATUS_OK;
        }

        status_t ComboBox::on_key_down(const ws::event_t *e)
        {
            switch (e->nCode)
            {
                case ws::WSK_UP:
                case ws::WSK_KEYPAD_UP:
                    if (scroll_item(-1, 1))
                        sSlots.execute(SLOT_SUBMIT, this, NULL);
                    break;

                case ws::WSK_DOWN:
                case ws::WSK_KEYPAD_DOWN:
                    if (scroll_item(1, 1))
                        sSlots.execute(SLOT_SUBMIT, this, NULL);
                    break;

                case ws::WSK_KEYPAD_ENTER:
                case ws::WSK_RETURN:
                case ws::WSK_KEYPAD_SPACE:
                case ' ':
                    sOpened.toggle();
                    break;

                default:
                    break;
            }

            return STATUS_OK;
        }

        status_t ComboBox::on_change()
        {
            return STATUS_OK;
        }

        status_t ComboBox::on_submit()
        {
            return STATUS_OK;
        }
    }
}



/*
 * StepFloat.cpp
 *
 *  Created on: 6 июн. 2020 г.
 *      Author: sadko
 */

#include <lsp-plug.in/tk/tk.h>

namespace lsp
{
    namespace tk
    {
        const prop::desc_t StepFloat::DESC[] =
        {
            { "",               PT_FLOAT    },
            { ".step",          PT_FLOAT    },
            { ".accel",         PT_FLOAT    },
            { ".decel",         PT_FLOAT    },
            { NULL,             PT_UNKNOWN  }
        };

        void StepFloat::Listener::notify(atom_t property)
        {
            pValue->commit(property);
        }

        StepFloat::StepFloat(prop::Listener *listener):
            MultiProperty(listener),
            sListener(this)
        {
            fStep       = 0.01f;
            fAccel      = 10.0f;
            fDecel      = 0.1f;
        }

        StepFloat::~StepFloat()
        {
            MultiProperty::unbind(vAtoms, DESC, &sListener);
        }

        void StepFloat::commit(atom_t property)
        {
            if ((pStyle == NULL) || (property < 0))
                return;

            float v;
            LSPString s;

            if ((property == vAtoms[P_STEP]) && (pStyle->get_float(vAtoms[P_STEP], &v) == STATUS_OK))
                fStep           = v;
            if ((property == vAtoms[P_ACCEL]) && (pStyle->get_float(vAtoms[P_ACCEL], &v) == STATUS_OK))
                fAccel          = v;
            if ((property == vAtoms[P_DECEL]) && (pStyle->get_float(vAtoms[P_DECEL], &v) == STATUS_OK))
                fDecel          = v;

            // Compound property
            if ((property == vAtoms[P_VALUE]) && (pStyle->get_string(vAtoms[P_VALUE], &s) == STATUS_OK))
            {
                float v[3];
                size_t n    = Property::parse_floats(v, 3, &s);
                switch (n)
                {
                    case 1:
                        fStep       = v[0];
                        fAccel      = 10.0f;
                        fDecel      = 0.1f;
                        break;
                    case 2:
                        fStep       = v[0];
                        fAccel      = v[1];
                        fDecel      = 1.0f / v[1];
                        break;
                    case 3:
                        fStep       = v[0];
                        fAccel      = v[1];
                        fDecel      = v[2];
                        break;
                    default:
                        break;
                }
            }


            if (pListener != NULL)
                pListener->notify(this);
        }

        void StepFloat::sync()
        {
            if (pStyle != NULL)
            {
                pStyle->begin(&sListener);
                {
                    LSPString s;

                    // Simple components
                    if (vAtoms[P_STEP] >= 0)
                        pStyle->set_float(vAtoms[P_STEP], fStep);
                    if (vAtoms[P_ACCEL] >= 0)
                        pStyle->set_float(vAtoms[P_ACCEL], fAccel);
                    if (vAtoms[P_DECEL] >= 0)
                        pStyle->set_float(vAtoms[P_DECEL], fDecel);

                    // Compound properties
                    s.fmt_ascii("%f %f %f", fStep, fAccel, fDecel);
                    if (vAtoms[P_VALUE] >= 0)
                        pStyle->set_string(vAtoms[P_VALUE], &s);
                }
                pStyle->end();
            }
            if (pListener != NULL)
                pListener->notify(this);
        }

        float StepFloat::set(float v)
        {
            float old = fStep;
            if (old == v)
                return old;

            fStep       = v;
            sync();
            return old;
        }

        void StepFloat::set(float v, float accel, float decel)
        {
            if ((fStep == v) &&
                (fAccel == accel) &&
                (fDecel == decel))
                return;

            fStep       = v;
            fAccel      = accel;
            fDecel      = decel;
            sync();
        }

        float StepFloat::set_accel(float v)
        {
            float old = fAccel;
            if (old == v)
                return old;

            fAccel      = v;
            sync();
            return old;
        }

        float StepFloat::set_decel(float v)
        {
            float old = fDecel;
            if (old == v)
                return old;

            fDecel      = v;
            sync();
            return old;
        }

        namespace prop
        {
            status_t StepFloat::init(Style *style, float step, float accel, float decel)
            {
                if (pStyle == NULL)
                    return STATUS_BAD_STATE;

                style->begin();
                {
                    LSPString s;

                    pStyle->create_float(vAtoms[P_STEP], fStep);
                    pStyle->create_float(vAtoms[P_ACCEL], fAccel);
                    pStyle->create_float(vAtoms[P_DECEL], fDecel);

                    // Compound properties
                    s.fmt_ascii("%f %f %f", fStep, fAccel, fDecel);
                    pStyle->create_string(vAtoms[P_VALUE], &s);
                }
                style->end();
                return STATUS_OK;
            }
        }
    } /* namespace tk */
} /* namespace lsp */



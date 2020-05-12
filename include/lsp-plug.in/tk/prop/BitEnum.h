/*
 * BitBitEnum.h
 *
 *  Created on: 8 мая 2020 г.
 *      Author: sadko
 */

#ifndef LSP_PLUG_IN_TK_PROP_BITENUM_H_
#define LSP_PLUG_IN_TK_PROP_BITENUM_H_

#ifndef LSP_PLUG_IN_TK_IMPL
    #error "use <lsp-plug.in/tk/tk.h>"
#endif

namespace lsp
{
    namespace tk
    {
        class Display;
        class Widget;

        /**
         * Integering property interface
         */
        class BitEnum: public Property
        {
            private:
                BitEnum & operator = (const BitEnum *);

            protected:
                class Listener: public IStyleListener
                {
                    protected:
                        BitEnum       *pValue;

                    public:
                        inline Listener(BitEnum *ptr)             { pValue = ptr;     }

                    public:
                        virtual void notify(atom_t property);
                };

            protected:
                atom_t              nAtom;
                size_t              nValue;
                Listener            sListener;
                const prop::enum_t *pEnum;

            protected:
                status_t            unbind();
                status_t            bind(atom_t property, Style *style);
                void                commit();
                void                sync();
                size_t              xset(size_t v);
                size_t              xset(size_t v, bool flag);
                size_t              xunset(size_t v);
                size_t              xtoggle(size_t v);
                size_t              xset_all(size_t v);

            protected:
                explicit BitEnum(const prop::enum_t *xenum, prop::Listener *listener = NULL);
                ~BitEnum();

        };
    }
}



#endif /* LSP_PLUG_IN_TK_PROP_BITENUM_H_ */

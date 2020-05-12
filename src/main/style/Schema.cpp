/*
 * Schema.cpp
 *
 *  Created on: 6 мая 2020 г.
 *      Author: sadko
 */

#include <lsp-plug.in/tk/tk.h>

namespace lsp
{
    namespace tk
    {
        
        Schema::Schema(Display *dpy)
        {
            pDisplay    = dpy;
        }
        
        Schema::~Schema()
        {
            destroy_context(&sCtx);
        }

        void Schema::destroy_context(context_t *ctx)
        {
            // Destroy color aliases
            lltl::parray<lsp::Color> colors;
            if (ctx->vColors.values(&colors))
            {
                for (size_t i=0, n=colors.size(); i<n; ++i)
                    delete colors.uget(i);
                colors.flush();
            }
            ctx->vColors.flush();

            // Destroy styles
            lltl::parray<style_t> styles;
            if (ctx->vStyles.values(&styles))
            {
                for (size_t i=0, n=styles.size(); i<n; ++i)
                    destroy_style(styles.uget(i));
                styles.flush();
            }
        }

        void Schema::destroy_style(style_t *s)
        {
            s->sStyle.destroy();
            for (size_t i=0, n=s->vParents.size(); i<n; ++i)
                delete s->vParents.uget(i);
            s->vParents.flush();
        }

        void Schema::init_context(context_t *s)
        {
            s->pRoot        = NULL;
        }

        status_t Schema::parse_file(const char *path, const char *charset)
        {
            xml::PullParser parser;
            status_t res = parser.open(path, charset);
            if (res == STATUS_OK)
                res = parse_document(&parser);
            if (res == STATUS_OK)
                res = parser.close();
            else
                parser.close();
            return res;
        }

        status_t Schema::parse_file(const LSPString *path, const char *charset)
        {
            xml::PullParser parser;
            status_t res = parser.open(path, charset);
            if (res == STATUS_OK)
                res = parse_document(&parser);
            if (res == STATUS_OK)
                res = parser.close();
            else
                parser.close();
            return res;
        }

        status_t Schema::parse_file(const io::Path *path, const char *charset)
        {
            xml::PullParser parser;
            status_t res = parser.open(path, charset);
            if (res == STATUS_OK)
                res = parse_document(&parser);
            if (res == STATUS_OK)
                res = parser.close();
            else
                parser.close();
            return res;
        }

        status_t Schema::parse_data(io::IInStream *is, size_t flags, const char *charset)
        {
            xml::PullParser parser;
            status_t res = parser.wrap(is, flags, charset);
            if (res == STATUS_OK)
                res = parse_document(&parser);
            if (res == STATUS_OK)
                res = parser.close();
            else
                parser.close();
            return res;
        }

        status_t Schema::parse_data(const char *str, const char *charset)
        {
            xml::PullParser parser;
            status_t res = parser.wrap(str, charset);
            if (res == STATUS_OK)
                res = parse_document(&parser);
            if (res == STATUS_OK)
                res = parser.close();
            else
                parser.close();
            return res;
        }

        status_t Schema::parse_data(const LSPString *str)
        {
            xml::PullParser parser;
            status_t res = parser.wrap(str);
            if (res == STATUS_OK)
                res = parse_document(&parser);
            if (res == STATUS_OK)
                res = parser.close();
            else
                parser.close();
            return res;
        }

        status_t Schema::parse_data(io::IInSequence *seq, size_t flags)
        {
            xml::PullParser parser;
            status_t res = parser.wrap(seq, flags);
            if (res == STATUS_OK)
                res = parse_document(&parser);
            if (res == STATUS_OK)
                res = parser.close();
            else
                parser.close();
            return res;
        }

        status_t Schema::parse_document(xml::PullParser *p)
        {
            status_t res;
            bool read = false;

            context_t ctx;
            init_context(&ctx);

            while ((res = p->read_next()) > 0)
            {
                if (res == xml::XT_END_DOCUMENT)
                    break;

                switch (res)
                {
                    case xml::XT_CHARACTERS:
                    case xml::XT_COMMENT:
                    case xml::XT_DTD:
                    case xml::XT_START_DOCUMENT:
                        break;

                    case xml::XT_START_ELEMENT:
                        if ((read) || (p->name() == NULL) || (!p->name()->equals_ascii("schema")))
                        {
                            destroy_context(&ctx);
                            return STATUS_CORRUPTED;
                        }

                        if ((res = parse_schema(p)) != STATUS_OK)
                        {
                            destroy_context(&ctx);
                            return res;
                        }

                        read = true;
                        break;

                    default:
                        return STATUS_CORRUPTED;
                }
            }

            if ((res < 0) && (res != -STATUS_EOF))
                return -res;
            if (!read)
                return STATUS_CORRUPTED;

            res = apply_context(&ctx);
            destroy_context(&ctx);

            return res;
        }

        status_t Schema::parse_schema(xml::PullParser *p)
        {
            status_t res;

            while ((res = p->read_next()) > 0)
            {
                switch (res)
                {
                    case xml::XT_CHARACTERS:
                    case xml::XT_COMMENT:
                        break;

                    case xml::XT_START_ELEMENT:
                        if (!p->name()->equals_ascii("colors"))
                            res = parse_colors(p);
                        else if (!p->name()->equals_ascii("style"))
                            res = parse_style(p, false);
                        else if (!p->name()->equals_ascii("root"))
                            res = parse_style(p, true);
                        else
                            return STATUS_CORRUPTED;
                        break;

                    case xml::XT_END_ELEMENT:
                        if (!p->name()->equals_ascii("schema"))
                            return STATUS_CORRUPTED;
                        return STATUS_OK;

                    default:
                        return STATUS_CORRUPTED;
                }
            }

            if ((res < 0) && (res != -STATUS_EOF))
                return -res;

            return STATUS_OK;
        }

        status_t Schema::parse_colors(xml::PullParser *p)
        {
            status_t res;

            while ((res = p->read_next()) > 0)
            {
                switch (res)
                {
                    case xml::XT_CHARACTERS:
                    case xml::XT_COMMENT:
                        break;

                    case xml::XT_START_ELEMENT:
                        break;

                    case xml::XT_END_ELEMENT:
                        if (!p->name()->equals_ascii("colors"))
                            return STATUS_CORRUPTED;
                        return STATUS_OK;

                    default:
                        return STATUS_CORRUPTED;
                }
            }

            if ((res < 0) && (res != -STATUS_EOF))
                return -res;

            return STATUS_OK;
        }

        status_t Schema::parse_style(xml::PullParser *p, bool root)
        {
            status_t res;

            while ((res = p->read_next()) > 0)
            {
                switch (res)
                {
                    case xml::XT_CHARACTERS:
                    case xml::XT_COMMENT:
                        break;

                    case xml::XT_START_ELEMENT:
                        if (!p->name()->equals_ascii("colors"))
                            res = parse_colors(p);
                        else if (!p->name()->equals_ascii("style"))
                            res = parse_style(p, false);
                        else if (!p->name()->equals_ascii("root"))
                            res = parse_style(p, true);
                        else
                            return STATUS_CORRUPTED;
                        break;

                    case xml::XT_END_ELEMENT:
                        if (!p->name()->equals_ascii("schema"))
                            return STATUS_CORRUPTED;
                        return STATUS_OK;

                    default:
                        return STATUS_CORRUPTED;
                }
            }

            if ((res < 0) && (res != -STATUS_EOF))
                return -res;

            return STATUS_OK;
        }

        status_t Schema::apply_context(context_t *ctx)
        {
            return STATUS_OK;
        }

    
    } /* namespace tk */
} /* namespace lsp */

#include "poppler/PDFDoc.h"
#include "goo/GooString.h"
#include "poppler/Link.h"

int main (int argc, char* argv[])
{
    // auto filename = GooString("/tmp/test.pdf");
    auto filename = GooString("/tmp/blah.pdf");
    auto document = new PDFDoc(&filename);

    Page* page = document->getPage(1);
    Annots* annotations = page->getAnnots();
    Links* links = new Links(annotations);

    for (int i = 0; i < links->getNumLinks(); ++i)
    {
        AnnotLink* link = links->getLink(i);
        LinkAction* action = link->getAction();
        LinkActionKind action_kind = action->getKind();

        printf("This link has action %d\n", action_kind);

        switch (action_kind)
        {
            case actionGoTo:
            {
                LinkGoTo* goto_link = static_cast<LinkGoTo*>(action);
                break;
            }
            case actionURI:
            {
                LinkURI* uri_link = static_cast<LinkURI*>(action);
                const GooString* uri = uri_link->getURI();
                // const char* cstring = uri->getCString();
                printf("This is a URI link that goes to: %s\n", uri->getCString());
                break;
            }
            default:
            {
                printf("I don't know what to do with that action!\n");
                break;
            }
        }
    }

    printf("Hello\n");
}

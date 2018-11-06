#include <stdio.h>

#include "poppler/PDFDoc.h"
#include "goo/GooString.h"
#include "poppler/Link.h"
#include <unordered_map>

int main (int argc, char* argv[])
{
    std::unordered_map<std::string, int> id_dictionary;

    for (int i = 2; i < argc; ++i)
    {
        printf("%s is on page %d\n", argv[i], i);
        id_dictionary[argv[i]] = i;
    }

    GooString filename = GooString(argv[1]);
    PDFDoc* document = new PDFDoc(&filename);

    Page* page = document->getPage(1);
    Annots* annotations = page->getAnnots();
    Links* links = new Links(annotations);

    for (int i = 0; i < links->getNumLinks(); ++i)
    {
        AnnotLink* link = links->getLink(i);
        LinkAction* action = link->getAction();
        LinkActionKind action_kind = action->getKind();

        switch (action_kind)
        {
            case actionURI:
            {
                LinkURI* uri_link = static_cast<LinkURI*>(action);
                const GooString* uri = uri_link->getURI();
                printf("This is a URI link that goes to: %s\n", uri->getCString());

                auto result = id_dictionary.find(uri->getCString());
                if ( result == id_dictionary.end() )
                {
                    printf("That target is not in the document!\n");
                    continue;
                }

                int target_page = result->second;
                printf("That target is on page %d\n", target_page);

                Array* destination_array = new Array(document->getXRef());

                // Indirect reference to page
                Page* destination_page = document->getPage(target_page);
                Ref destination_page_reference = destination_page->getRef();
                destination_array->add(Object(destination_page_reference.num, destination_page_reference.gen));
                destination_array->add(Object(objName, "XYZ"));
                destination_array->add(Object(objNull));
                destination_array->add(Object(objNull));
                destination_array->add(Object(objNull));

                PDFRectangle* original_rectangle = link->getRect();
                PDFRectangle new_rectangle = PDFRectangle(
                    original_rectangle->x1, 
                    original_rectangle->y1, 
                    original_rectangle->x2, 
                    original_rectangle->y2);
                Array* rectangle_array = new Array(document->getXRef());
                rectangle_array->add(Object(new_rectangle.x1));
                rectangle_array->add(Object(new_rectangle.y1));
                rectangle_array->add(Object(new_rectangle.x2));
                rectangle_array->add(Object(new_rectangle.y2));

                Dict* annotation_dictionary = new Dict(document->getXRef());
                annotation_dictionary->set("Type", Object(objName, "Annot"));
                annotation_dictionary->set("Subtype", Object(objName, "Link"));
                annotation_dictionary->set("Rect", Object(rectangle_array));

                Dict* action_dictionary = new Dict(document->getXRef());
                action_dictionary->set("S", Object(objName, "GoTo"));
                action_dictionary->set("D", Object(destination_array));
                annotation_dictionary->set("A", Object(action_dictionary));

                Array* border_array = new Array(document->getXRef());
                border_array->add(Object(0));
                border_array->add(Object(0));
                border_array->add(Object(0));
                annotation_dictionary->set("Border", Object(border_array));

                Object* annotation_dictionary_object = new Object(annotation_dictionary);

                Ref ref = document->getXRef()->addIndirectObject(annotation_dictionary_object);
                Object* reference_object = new Object(ref.num, ref.gen);

                AnnotLink* new_link = new AnnotLink(document, annotation_dictionary_object, reference_object);
                
                page->removeAnnot(link);
                page->addAnnot(new_link);

                break;
            }
            default:
            {
                printf("I don't know what to do with that action!\n");
                break;
            }
        }
    }

    auto output_filename = GooString(argv[argc - 1]);
    document->saveAs(&output_filename);

    printf("Done!\n");
}

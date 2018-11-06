#include "poppler/PDFDoc.h"
#include "goo/GooString.h"
#include "poppler/Link.h"

int main (int argc, char* argv[])
{
    auto filename = GooString("/tmp/test.pdf");
    // auto filename = GooString("/tmp/blah.pdf");
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
            // I want to change all GoTo's to be LinkURI's that go to
            // Google
            case actionGoTo:
            {
                LinkGoTo* goto_link = static_cast<LinkGoTo*>(action);
                const GooString* destination_name = goto_link->getNamedDest();
                printf("This is a GoTo link that goes to: %s\n", destination_name->getCString());

                // GooString* uri_string = new GooString("http://www.google.com");
                // Object uri_string_object = Object(uri_string);
                // GooString base_uri_string = GooString("");
                // LinkURI* new_link_action = new LinkURI(&uri_string_object, &base_uri_string);

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

                // A needs to be a dict
                // S: URI
                // URI: The destination
                Dict* action_dictionary = new Dict(document->getXRef());
                GooString* fuck_you = new GooString("URI");
                action_dictionary->set("S", Object(objName, "URI"));
                GooString* fuck_you_too = new GooString("http://www.google.com");
                action_dictionary->set("URI", Object(fuck_you_too));
                
                annotation_dictionary->set("A", Object(action_dictionary));
                // annotation_dictionary->set("P", Object(page->getNum()));

                Object* dummy_object = new Object();
                Object* annotation_dictionary_object = new Object(annotation_dictionary);


                Ref ref = document->getXRef()->addIndirectObject(annotation_dictionary_object);
                Object* reference_object = new Object(ref.num, ref.gen);

                annotation_dictionary_object->print(stdout);
                printf("\n");
                AnnotLink* new_link = new AnnotLink(document, annotation_dictionary_object, reference_object);

                
                page->removeAnnot(link);
                page->addAnnot(new_link);

                // Ref reference = link->getRef();
                // printf("Found a reference number %d\n", reference.num);
                // auto reference_entry = document->getXRef()->getEntry(reference.num);

                // FILE* my_file = fopen("/tmp/test-output", "w");
                // FileOutStream* output_stream = new FileOutStream(my_file, 0);
                // document->getXRef()->writeTableToFile(output_stream, true);
                // output_stream->close();
                

                break;
            }
            case actionURI:
            {
                // LinkURI* uri_link = static_cast<LinkURI*>(action);
                // const GooString* uri = uri_link->getURI();
                // printf("This is a URI link that goes to: %s\n", uri->getCString());
                break;
            }
            default:
            {
                printf("I don't know what to do with that action!\n");
                break;
            }
        }
    }

    auto output_filename = GooString("./output.pdf");
    document->saveAs(&output_filename);

    printf("Hello\n");
}

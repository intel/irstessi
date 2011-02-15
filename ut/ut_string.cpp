// ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab:

#include <iostream>

#include "exception.h"
#include "string.h"

using namespace std;

int main(int argc, char *argv[])
{
    char buf[2048];

    cout << endl << "== Create static object =============" << endl;
    String t;

    cout << "'" << t.get() << "'";
    if (!t) {
        cout << " (empty)";
    }
    cout << endl;
    cout << "t.length() == " << t.length() << endl;
    cout << "t.size() == " << t.size() << endl;
    cout << "t.capacity() == " << t.capacity() << endl;

    cout << endl << "== Assignment to the object =========" << endl;
    t = "Big brow fox jumps over the lasy dog.";

    cout << "'" << t.get() << "'";
    if (!t) {
        cout << " (empty)";
    }
    cout << endl;
    cout << "t.length() == " << t.length() << endl;
    cout << "t.size() == " << t.size() << endl;
    cout << "t.capacity() == " << t.capacity() << endl;

    cout << endl << "== Appending some UTF-8 characters ==" << endl;
    t += " [ółśćź]";

    cout << "'" << t.get() << "'";
    if (!t) {
        cout << " (empty)";
    }
    cout << endl;
    cout << "t.length() == " << t.length() << endl;
    cout << "t.size() == " << t.size() << endl;
    cout << "t.capacity() == " << t.capacity() << endl;

    cout << endl << "== Assiging a long long number ======" << endl;
    t = -1000LL;

    cout << "'" << t.get() << "'" << endl;
    cout << "t.length() == " << t.length() << endl;
    cout << "t.size() == " << t.size() << endl;
    cout << "t.capacity() == " << t.capacity() << endl;

    cout << endl << "== Assigning an unsigned long long ==" << endl;
    t = 1000ULL;

    cout << "'" << t.get() << "'" << endl;
    cout << "t.length() == " << t.length() << endl;
    cout << "t.size() == " << t.size() << endl;
    cout << "t.capacity() == " << t.capacity() << endl;

    cout << endl << "== Appending a string ===============" << endl;
    t.append(" + ");

    cout << "'" << t.get() << "'" << endl;
    cout << "t.length() == " << t.length() << endl;
    cout << "t.size() == " << t.size() << endl;
    cout << "t.capacity() == " << t.capacity() << endl;

    cout << endl << "== Appending a negative number ======" << endl;
    t.append(-1000);

    cout << "'" << t.get() << "'" << endl;
    cout << "t.length() == " << t.length() << endl;
    cout << "t.size() == " << t.size() << endl;
    cout << "t.capacity() == " << t.capacity() << endl;

    cout << endl << "== Appending a text =================" << endl;
    t.append(" = 0");

    cout << "'" << t.get() << "'" << endl;
    cout << "t.length() == " << t.length() << endl;
    cout << "t.size() == " << t.size() << endl;
    cout << "t.capacity() == " << t.capacity() << endl;

    cout << endl << "== Comparing ASCII strings (equal) ==" << endl;
    String l("Brow fox jumped over the dog");
    String r(l);

    cout << "'" << l.get() << "' == '" << r.get() << "'";
    if (l == r) {
        cout << " ...PASSED (true)";
    } else {
        cout << " ?!? <bug> ?!?";
    }
    cout << endl;

    cout << endl << "== Comparing ASCII strings (diff) ===" << endl;
    r = "Fox which is brown jumped over the lazy dog";

    cout << "'" << l.get() << "' != '" << r.get() << "'";
    if (l != r) {
        cout << " ...PASSED (true)";
    } else {
        cout << " ?!? <bug> ?!?";
    }
    cout << endl;

    cout << endl << "== Comparing ASCII strings (same) ===" << endl;
    cout << "'" << l.get() << "' == '" << l.get() << "'";
    if (l == l) {
        cout << " ...PASSED (true)";
    } else {
        cout << " ?!? <bug> ?!?";
    }
    cout << endl;

    cout << "'" << l.get() << "' != '" << l.get() << "'";
    if (l != l) {
        cout << " ?!? <bug> ?!?";
    } else {
        cout << " ...PASSED (false)";
    }
    cout << endl;

    cout << endl << "== Compare UTF-8 strings (equal) ====" << endl;
    l = "Świętość świącionośna";
    r = l;

    cout << "'" << l.get() << "' == '" << r.get() << "'";
    if (l == r) {
        cout << " ...PASSED (true)";
    } else {
        cout << " ?!? <bug> ?!?";
    }
    cout << endl;
    cout << "'" << l.get() << "' != '" << r.get() << "'";
    if (l != r) {
        cout << " ?!? <bug> ?!?";
    } else {
        cout << " ...PASSED (false)";
    }
    cout << endl;

    cout << endl << "== Compare UTF-8 string (diff) ======" << endl;
    r = "Świętość swięcionośna";

    cout << "'" << l.get() << "' == '" << r.get() << "'";
    if (l == r) {
        cout << " ?!? <bug> ?!?";
    } else {
        cout << " ...PASSED (false)";
    }
    cout << endl;

    cout << "'" << l.get() << "' != '" << r.get() << "'";
    if (l != r) {
        cout << " ...PASSED (true)";
    } else {
        cout << " ?!? <bug> ?!?";
    }
    cout << endl;

    cout << endl << "== Getting the content to buffer ====" << endl;
    r.get(buf, sizeof(buf));

    cout << "'" << buf << "'" << endl;

    cout << endl << "== Content to buffer (3 last chars) =" << endl;
    r.get(buf, sizeof(buf), 18);

    cout << "'" << buf << "'" << endl;

    cout << endl << "== Others ===================" << endl;
    l = r.left("swięci");
    cout << "r.left('swięci') = '" << l.get() << "'" << endl;
    l = r.right("ęto");
    cout << "r.right('ęto') = '" << l.get() << "'" << endl;

    cout << endl;
    return 0;
}

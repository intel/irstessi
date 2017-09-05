/*
 * Copyright 2011 - 2017 Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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

    String t1("internal:imsm");
    String t2("internal:/md127/0");
    String t3("/sys/block/pci0000:00/0000:00:1f.2");
    String t4("Ślad/ółwś/zaginął/niepor-ówi=/nywalnie=/głęboko/ółwś");
    String t5(" \n\t\rŚkoda\n$test\n\t\v     ");
    String t6("  \t Śkoda słów...ęł\n\t    \t");
    String t7("3.0\n");

    cout << endl << "---- t1 = '" << t1.get() << "'" << endl;
    cout << "t1.left(':') = '" << t1.left(":").get() << "' [internal]" << endl;
    cout << "t1.reverse_left(':') = '" << t1.reverse_left(":").get() << "' [internal]" << endl;

    cout << endl << "---- t2 = '" << t2.get() << "'" << endl;
    cout << "t2.right('/') = '" << t2.right("/").get() << "' [/md127/0]" << endl;
    cout << "t2.reverse_right('/') = '" << t2.reverse_right("/").get() << "' [/0]" << endl;
    cout << "t2.after('/') = '" << t2.after("/").get() << "' [md127/0]" << endl;
    cout << "t2.reverse_after('/') = '" << t2.reverse_after("/").get() << "' [0]" << endl;

    cout << endl << "---- t3 = '" << t3.get() << "'" << endl;
    cout << "t3.reverse_after('/') = " << t3.reverse_after("/").get() << "'" << endl;
    cout << "t3.right('0000:') = '" << t3.right("0000:").get() << "'" << endl;
    cout << "t3.reverse_right('0000:') = '" << t3.reverse_right("0000:").get() << "'" << endl;

    cout << endl << "---- t4 = '" << t4.get() << "'" << endl;
    cout << "t4.left('=/') = '" << t4.left("=/").get() << "' [Ślad/ółwś/zaginął/niepor-ówi]" << endl;
    cout << "t4.right('=/') = '" << t4.right("=/").get() << "' [=/nywalnie=/głęboko/ółwś]" << endl;
    cout << "t4.reverse_left('=/') = '" << t4.reverse_left("=/").get() << "' [Ślad/ółwś/zaginął/niepor-ówi=/nywalnie]" << endl;
    cout << "t4.reverse_right('=/') = '" << t4.reverse_right("=/").get() << "' [=/głęboko/ółwś]" << endl;

    cout << "t4.length() = '" << t4.length() << "'" << endl;
    cout << "t4.size() = '" << t4.size() << "'" << endl;

    cout << endl << "---- t5 = '" << t5.get() << "'" << endl;
    t5.trim();
    cout << "t5.trim() = '" << t5.get() << "'" << endl;

    cout << endl << "---- t6 = '" << t6.get() << "'" << endl;
    t6.trim();
    cout << "t6.trim() = '" << t6.get() << "'" << endl;

    cout << endl << "---- t7 = '" << t7.get() << "'" << endl;
    t7.trim();
    cout << "t7.trim() = '" << t7.get() << "'" << endl;

    cout << endl;
    return 0;
}

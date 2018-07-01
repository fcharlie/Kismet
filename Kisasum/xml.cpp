#include "console.hpp"
#include "kisasum.h"
#include <sstream>

//<? xml version = "1.0">
//<algorithm>md5</algorithm>
//<files>
//  <file>
//    <name>filename</name>
//    <hash>xxxx</hash>
//  </file>
//</files>

/*
<?xml version="1.0" encoding="UTF-8"?><root>
<algorithm>SHA256</algorithm>
<files>
<name>powershell-6.0.0-beta.6-osx.10.12-x64.pkg</name>
<hash>E69C22F2224707223F32607D08F73C0BEADC3650069C49FC11C8071D08843309</hash>
</files>
<files>
<name>PowerShell-6.0.0-beta.6-win10-win2016-x64.msi</name>
<hash>2F0F6F030F254590C63CD47CC3C4CD1952D002639D2F27F37E616CD2A5DDE84C</hash>
</files>
</root>

*/

int KisasumPrintXML(const KisasumResult &result) {
  std::wstring ws(LR"(<?xml version="1.0">)");
  ws.append(L"\n<root>\n  <algorithm>")
      .append(result.algorithm)
      .append(L"</algorithm>\n  <files>\n");
  for (const auto &e : result.elems) {
    ws.append(L"    <file>\n      <name>")
        .append(e.filename)
        .append(L"</name>\n      <hash>")
        .append(e.hash)
        .append(L"</hash>\n    </file>\n");
  }
  ws.append(L"  </files>\n</root>\n");
  console::WriteFormatted(ws.data(), ws.size());
  return 0;
}
#include "console.hpp"
#include "kisasum.h"

/*
{
  "algorithm":"md5",
  "file":[
     {
	  "name":"filename",
	   "hash":"hash"
	  },
    ]
}
*/
int KisasumPrintJSON(const KisasumResult &result) {
	std::wstring ws(L"{\n"); /// start
	ws.append(L"  \"algorithm\": \"")
		.append(result.algorithm)
		.append(L"\",\n  \"files\": \n  [");
	for (const auto &e : result.elems) {
		ws.append(L"\n    {\n      \"name\": \"")
			.append(e.filename)
			.append(L"\",\n      \"hash\": \"")
			.append(e.hash)
			.append(L"\"\n    },");
	}
	if (ws.back() == L',') {
		ws.pop_back();
	}
	ws.append(L"\n  ]\n}\n");
	console::WriteFormatted(ws.data(), ws.size());
	return 0;
}
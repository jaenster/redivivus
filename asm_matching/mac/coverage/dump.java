// READ-ONLY dump of the clean Mac program's functions for coverage.ts --import. No transaction is opened.
// Run it with ghidra-mcp execute_script (language java, sandbox false) in a read-only session on
// Diablo2Lod/mac/intel/1.14d/DiabloII_macho (never _legacy). The worker cannot write to this machine, so
// the result goes to a daemon upload slot: make one with request_upload, put its id in for @SLOT@, run,
// then fetch https://<daemon>/upload/<id>/raw.
// Columns: addr, end (one past the body's highest address), bytes (body size), thunk ("T" for an import
// stub), the qualified name, and the __FILE__ path literals the function references ("|"-joined, below
// Diablo2/Source with the leading "../" dropped).
ghidra.program.model.listing.Listing L = currentProgram.getListing(); ghidra.program.model.symbol.ReferenceManager rm = currentProgram.getReferenceManager(); ghidra.program.model.listing.FunctionManager fm = currentProgram.getFunctionManager();
java.util.HashMap<Long,java.util.TreeSet<String>> files = new java.util.HashMap<>();
ghidra.program.model.listing.DataIterator it = L.getDefinedData(true);
while (it.hasNext()) { ghidra.program.model.listing.Data d = it.next(); Object v = d.getValue(); if (!(v instanceof String)) continue; String s=(String)v;
 if (!s.matches("^[^\\n]*/[^/\\n]+\\.(cpp|CPP|c|C|h|H|hpp|mm|m)$")) continue;
 String p = s.replaceFirst("^.*/DiabloAll/", "").replaceAll("^(\\.\\./)+", "");
 for (ghidra.program.model.symbol.Reference r : rm.getReferencesTo(d.getAddress())) { ghidra.program.model.listing.Function f = fm.getFunctionContaining(r.getFromAddress()); if (f!=null) files.computeIfAbsent(f.getEntryPoint().getOffset(), k->new java.util.TreeSet<>()).add(p); } }
StringBuilder sb = new StringBuilder("#program " + currentProgram.getDomainFile().getPathname() + " v" + currentProgram.getDomainFile().getVersion() + "\naddr\tend\tbytes\tthunk\tname\tfile_literal\n");
int n=0;
for (ghidra.program.model.listing.Function f : fm.getFunctions(true)) { if (f.isExternal()) continue; long a=f.getEntryPoint().getOffset(); ghidra.program.model.address.AddressSetView b=f.getBody();
 java.util.TreeSet<String> fl = files.get(a);
 sb.append(String.format("%08x\t%08x\t%d\t%s\t%s\t%s\n", a, b.getMaxAddress().getOffset()+1, b.getNumAddresses(), f.isThunk()?"T":"", f.getName(true), fl==null?"":String.join("|", fl))); n++; }
byte[] bytes = sb.toString().getBytes("UTF-8");
String base = "http://" + System.getenv("GHIDRA_MCP_SERVICE_HOST") + ":" + System.getenv("GHIDRA_MCP_SERVICE_PORT");
var h = (java.net.HttpURLConnection) new java.net.URL(base + "/upload/@SLOT@").openConnection();
h.setRequestMethod("PUT"); h.setDoOutput(true); h.getOutputStream().write(bytes);
println("rows " + n + " PUT " + h.getResponseCode() + " " + bytes.length);

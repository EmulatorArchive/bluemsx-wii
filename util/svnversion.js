/*
 * Prints the subversion revision number of the current directory.
 * Prints zero on error.
 * May break in future versions of subversion!
 */
var doc, nodes, sysenv;
var ForReading = 1;
var entriesPath = ".svn\\entries"
	
/* Earlier SVN clients used XML for the entries document */
function format4revision() {
    doc = new ActiveXObject("MSXML2.DOMDocument");
    doc.load(entriesPath);
    nodes = doc.selectNodes('//entry[@name=""]');
    if (nodes.length != 1)
	throw new Error();
    return nodes[0].getAttribute("revision");
}

/* After format 7, the 4th line of the entries file is the revision */
function format7revision() {
    fso = new ActiveXObject("Scripting.FileSystemObject");
    f = fso.openTextFile(entriesPath, ForReading, false);
    format = f.ReadLine();
    if (Number(format) < 7) throw new Error();
    f.ReadLine();
    f.ReadLine();
    rline = f.ReadLine();
    f.close();
    return Number(rline);
}

try {
    revision = format4revision();
} catch (e) { 
    try {
	revision = format7revision();
    } catch (e) {
	revision = "0";
    }
}
WScript.Echo(revision);

/*
 * iLand management hook: run an external R analysis once per simulation year.
 *
 * Configure this file as the management script in the project XML.
 * The callback onYearEnd() is executed once each year by iLand.
 */

// Change this if Rscript is not on PATH.
var rscriptExecutable = "Rscript";

// Path is relative to the project root (same base as this JS file).
var rScriptPath = "scripts/process_output_placeholder.R";

function shellQuotePosix(value) {
    // Safe single-quote escaping for POSIX shells.
    return "'" + String(value).replace(/'/g, "'\\''") + "'";
}

function runYearlyRScript(year) {
    var outputDir = Globals.path("output");
    var sentinel = "__ILAND_R_OK__";

    var cmd = shellQuotePosix(rscriptExecutable) + " " +
        shellQuotePosix(rScriptPath) + " " +
        shellQuotePosix(String(year)) + " " +
        shellQuotePosix(String(outputDir)) +
        " && echo " + shellQuotePosix(sentinel);

    console.log("Running yearly R analysis for simulation year " + year + "...");
    var out = Globals.systemCmd(cmd);

    if (String(out).indexOf(sentinel) === -1) {
        throw new Error(
            "Yearly R analysis failed for year " + year +
            ". Check the iLand log for stderr/stdout from R."
        );
    }

    console.log("Yearly R analysis finished for simulation year " + year + ".");
}

function onYearEnd() {
    runYearlyRScript(Globals.year);
}

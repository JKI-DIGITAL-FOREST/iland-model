#!/usr/bin/env Rscript

# Placeholder script for yearly post-processing of iLand outputs.
# Args:
#   1) simulation year (integer)
#   2) iLand output directory

args <- commandArgs(trailingOnly = TRUE)
if (length(args) < 2) {
  stop("Expected arguments: <year> <output_dir>")
}

year <- as.integer(args[[1]])
output_dir <- args[[2]]

if (is.na(year)) {
  stop("Invalid year argument: ", args[[1]])
}

cat(sprintf("[R] Start analysis for simulation year %d\n", year))
cat(sprintf("[R] Output directory: %s\n", output_dir))

if (!dir.exists(output_dir)) {
  stop("Output directory does not exist: ", output_dir)
}

# Example placeholder: find SQLite files that could hold iLand outputs.
sqlite_files <- list.files(output_dir, pattern = "\\.sqlite$", full.names = TRUE)
if (length(sqlite_files) == 0) {
  warning("No .sqlite files found in output directory.")
} else {
  cat("[R] Found SQLite file(s):\n")
  for (f in sqlite_files) {
    cat(sprintf("  - %s\n", f))
  }
}

# Placeholder for your future calculations, e.g. with RSQLite + dplyr.
# library(RSQLite)
# con <- DBI::dbConnect(RSQLite::SQLite(), dbname = sqlite_files[[1]])
# on.exit(DBI::dbDisconnect(con), add = TRUE)
# stand <- DBI::dbReadTable(con, "stand")
# ... your yearly metrics ...

cat(sprintf("[R] Finished analysis for simulation year %d\n", year))

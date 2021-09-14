#!/usr/local/bin/tclsh

set IROOT $env(DS1_INFRASTRUCTURE_ROOT)

set program $IROOT/utils/src/ipc-test/.bin/sparc-sunos4/Gcc_27x/Fast/marshallTest

set f [open "ds1-msgs.formats" "r"]

set line [gets $f]

exec /bin/rm -f marshallTest.results

while ([string compare $line ""]!=0) {
    # repeat this until reach end of file:
    if ([string compare $line ""]==0) break;
    set command [string range $line 4 [string length $line]]
    set arg [gets $f]

    exec echo "\n$command" >> marshallTest.results
    exec $program "$arg" >> marshallTest.results

    set line [gets $f]
}

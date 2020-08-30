clear

echo "---------------------------------------"
echo "        Ramayana : Code Metrics"
echo "---------------------------------------"
echo

echo -n "Total Lines of Code (Including OpenGLToolsLibrary) = "
grep "$" *.cpp *.h gltools/*.cpp gltools/*.h -c | sed "s/[a-zA-Z0-9_\.\/]*://" | awk "{ total = total + \$1 } END { print total }"

echo -n "Total Lines of Code (Only in Project) = "
grep "$" *.cpp *.h -c | sed "s/[a-zA-Z0-9_\.\/]*://" | awk "{ total = total + \$1 } END { print total }"

echo

echo -n "Number of .CPP files = "
ls *.cpp gltools/*.cpp | wc -l

echo -n "Number of .H files = "
ls *.h gltools/*.h | wc -l

echo
echo


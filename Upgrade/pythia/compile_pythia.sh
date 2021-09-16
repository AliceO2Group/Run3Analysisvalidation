g++ ${1}.cc \
 -O2 -ansi -W -Wall -std=c++11 -Wshadow -m64 -Wno-shadow \
 -o ${1}.exe \
     -I$PY8_HEPMC3_INCLUDE -L$PY8_HEPMC3_LIB -lHepMC3\
         -I$PY8_PYTHIA8_INCLUDE -L$PY8_PYTHIA8_LIB -lpythia8 \
     -I$PY8_ROOT_INCLUDE -L$Py8_ROOT_LIB -EG -lz \
     -I$PY8_FASTJET_INCLUDE -L$PY8_FASTJET_LIB -lfastjet -lRecursiveTools -lfastjettools  -lNsubjettiness  \
     -L$PY8_YAMLCPP_LIB -I$PY8_YAMLCPP_INCLUDE -lyaml-cpp \
     $PY8_FLAGS

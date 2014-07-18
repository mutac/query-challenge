#!/bin/bash

ROOT=`pwd`

mkdir -p bin
cd bin
cmake ../src -G "Unix Makefiles" -DEXAMPLE_DIR:STRING="$ROOT/examples"

cd ..
cat > build.sh <<END
#!/bin/bash
cd bin
make
END

chmod +x build.sh


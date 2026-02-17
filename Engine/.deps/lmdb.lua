project "lmdb"
    kind "StaticLib"
    language "C"
    staticruntime "Off"

    objdir ("%{RootPath}/build/Engine/obj")
    targetdir ("%{RootPath}/build/Engine/lib")

    files {
        "%{VendorPaths.lmdb}/libraries/liblmdb/mdb.c",
        "%{VendorPaths.lmdb}/libraries/liblmdb/midl.c",
        "%{VendorPaths.lmdb}/libraries/liblmdb/mdb_stat.c",
        "%{VendorPaths.lmdb}/libraries/liblmdb/mdb_copy.c",
        "%{VendorPaths.lmdb}/libraries/liblmdb/mdb_load.c",
        "%{VendorPaths.lmdb}/libraries/liblmdb/mdb_dump.c",
        "%{VendorPaths.lmdb}/libraries/liblmdb/mdb_drop.c",
    }

    includedirs {
        "%{Includes.lmdb}",
    }

    defines {

    }

    buildoptions {
        -- "-Wunused-result"
    }
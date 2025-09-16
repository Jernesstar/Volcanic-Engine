project "lmdb"
    kind "StaticLib"
    language "C"
    staticruntime "Off"

    objdir ("%{RootPath}/build/%{_ACTION}/Magma/obj")
    targetdir ("%{RootPath}/build/%{_ACTION}/Magma/lib")

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
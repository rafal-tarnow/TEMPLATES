TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lGLESv2
LIBS += -lglfw

######## COPY FILES FORM DATA DIRECTORY DO BUILD DIRECTORY #####
copydata.commands = $(COPY_DIR) -r $$PWD/data $$OUT_PWD
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata
################################################################

INCLUDEPATH += $$PWD/include

QMAKE_CFLAGS += -DFT2_BUILD_LIBRARY

SOURCES += \
    src/autofit/afangles.c \
    src/autofit/afblue.c \
    src/autofit/afcjk.c \
    src/autofit/afdummy.c \
    src/autofit/afglobal.c \
    src/autofit/afhints.c \
    src/autofit/afindic.c \
    src/autofit/aflatin.c \
    src/autofit/aflatin2.c \
    src/autofit/afloader.c \
    src/autofit/afmodule.c \
    src/autofit/afpic.c \
    src/autofit/afranges.c \
    src/autofit/afshaper.c \
    src/autofit/afwarp.c \
    src/autofit/autofit.c \
    src/base/basepic.c \
    src/base/ftadvanc.c \
    src/base/ftapi.c \
    src/base/ftbase.c \
    src/base/ftbbox.c \
    src/base/ftbdf.c \
    src/base/ftbitmap.c \
    src/base/ftcalc.c \
    src/base/ftcid.c \
    src/base/ftdbgmem.c \
    src/base/ftdebug.c \
    src/base/ftfntfmt.c \
    src/base/ftfstype.c \
    src/base/ftgasp.c \
    src/base/ftgloadr.c \
    src/base/ftglyph.c \
    src/base/ftgxval.c \
    src/base/fthash.c \
    src/base/ftinit.c \
    src/base/ftlcdfil.c \
    src/base/ftmac.c \
    src/base/ftmm.c \
    src/base/ftobjs.c \
    src/base/ftotval.c \
    src/base/ftoutln.c \
    src/base/ftpatent.c \
    src/base/ftpfr.c \
    src/base/ftpic.c \
    src/base/ftrfork.c \
    src/base/ftsnames.c \
    src/base/ftstream.c \
    src/base/ftstroke.c \
    src/base/ftsynth.c \
    src/base/ftsystem.c \
    src/base/fttrigon.c \
    src/base/fttype1.c \
    src/base/ftutil.c \
    src/base/ftwinfnt.c \
    src/base/md5.c \
#    src/bdf/bdf.c \
#    src/bdf/bdfdrivr.c \
#    src/bdf/bdflib.c \
    src/bzip2/ftbzip2.c \
    src/cache/ftcache.c \
    src/cache/ftcbasic.c \
    src/cache/ftccache.c \
    src/cache/ftccmap.c \
    src/cache/ftcglyph.c \
    src/cache/ftcimage.c \
    src/cache/ftcmanag.c \
    src/cache/ftcmru.c \
    src/cache/ftcsbits.c \
#    src/cff/cf2arrst.c \
#    src/cff/cf2blues.c \
#    src/cff/cf2error.c \
#    src/cff/cf2font.c \
#    src/cff/cf2ft.c \
#    src/cff/cf2hints.c \
#    src/cff/cf2intrp.c \
#    src/cff/cf2read.c \
#    src/cff/cf2stack.c \
#    src/cff/cff.c \
#    src/cff/cffcmap.c \
#    src/cff/cffdrivr.c \
#    src/cff/cffgload.c \
#    src/cff/cffload.c \
#    src/cff/cffobjs.c \
#    src/cff/cffparse.c \
#    src/cff/cffpic.c \
#    src/cid/cidgload.c \
#    src/cid/cidload.c \
#    src/cid/cidobjs.c \
#    src/cid/cidparse.c \
#    src/cid/cidriver.c \
#    src/cid/type1cid.c \
    src/gzip/adler32.c \
    src/gzip/ftgzip.c \
    src/gzip/infblock.c \
    src/gzip/infcodes.c \
    src/gzip/inflate.c \
    src/gzip/inftrees.c \
    src/gzip/infutil.c \
    src/gzip/zutil.c \
    src/lzw/ftlzw.c \
    src/lzw/ftzopen.c \
    src/otvalid/otvalid.c \
    src/otvalid/otvbase.c \
    src/otvalid/otvcommn.c \
    src/otvalid/otvgdef.c \
    src/otvalid/otvgpos.c \
    src/otvalid/otvgsub.c \
    src/otvalid/otvjstf.c \
    src/otvalid/otvmath.c \
    src/otvalid/otvmod.c \
#    src/pcf/pcf.c \
#    src/pcf/pcfdrivr.c \
#    src/pcf/pcfread.c \
#    src/pcf/pcfutil.c \
#    src/pfr/pfr.c \
#    src/pfr/pfrcmap.c \
#    src/pfr/pfrdrivr.c \
#    src/pfr/pfrgload.c \
#    src/pfr/pfrload.c \
#    src/pfr/pfrobjs.c \
#    src/pfr/pfrsbit.c \
    src/psaux/afmparse.c \
    src/psaux/psaux.c \
    src/psaux/psauxmod.c \
    src/psaux/psconv.c \
    src/psaux/psobjs.c \
    src/psaux/t1cmap.c \
    src/psaux/t1decode.c \
    src/pshinter/pshalgo.c \
    src/pshinter/pshglob.c \
    src/pshinter/pshinter.c \
    src/pshinter/pshmod.c \
    src/pshinter/pshpic.c \
    src/pshinter/pshrec.c \
    src/psnames/psmodule.c \
    src/psnames/psnames.c \
    src/psnames/pspic.c \
    src/raster/ftraster.c \
    src/raster/ftrend1.c \
    src/raster/raster.c \
    src/raster/rastpic.c \
    src/sfnt/pngshim.c \
    src/sfnt/sfdriver.c \
    src/sfnt/sfnt.c \
    src/sfnt/sfntpic.c \
    src/sfnt/sfobjs.c \
    src/sfnt/ttbdf.c \
    src/sfnt/ttcmap.c \
    src/sfnt/ttkern.c \
    src/sfnt/ttload.c \
    src/sfnt/ttmtx.c \
    src/sfnt/ttpost.c \
    src/sfnt/ttsbit.c \
    src/smooth/ftgrays.c \
    src/smooth/ftsmooth.c \
    src/smooth/ftspic.c \
    src/smooth/smooth.c \
    src/truetype/truetype.c \
    src/truetype/ttdriver.c \
    src/truetype/ttgload.c \
    src/truetype/ttgxvar.c \
    src/truetype/ttinterp.c \
    src/truetype/ttobjs.c \
    src/truetype/ttpic.c \
    src/truetype/ttpload.c \
    src/truetype/ttsubpix.c \
#    src/type1/t1afm.c \
#    src/type1/t1driver.c \
#    src/type1/t1gload.c \
#    src/type1/t1load.c \
#    src/type1/t1objs.c \
#    src/type1/t1parse.c \
#    src/type1/type1.c \
#    src/type42/t42drivr.c \
#    src/type42/t42objs.c \
#    src/type42/t42parse.c \
#    src/type42/type42.c \
  #  src/winfonts/winfnt.c \
    main.cpp


HEADERS += \
    src/autofit/afangles.h \
    src/autofit/afblue.cin \
    src/autofit/afblue.h \
    src/autofit/afblue.hin \
    src/autofit/afcjk.h \
    src/autofit/afcover.h \
    src/autofit/afdummy.h \
    src/autofit/aferrors.h \
    src/autofit/afglobal.h \
    src/autofit/afhints.h \
    src/autofit/afindic.h \
    src/autofit/aflatin.h \
    src/autofit/aflatin2.h \
    src/autofit/afloader.h \
    src/autofit/afmodule.h \
    src/autofit/afpic.h \
    src/autofit/afranges.h \
    src/autofit/afscript.h \
    src/autofit/afshaper.h \
    src/autofit/afstyles.h \
    src/autofit/aftypes.h \
    src/autofit/afwarp.h \
    src/autofit/afwrtsys.h \
    src/base/basepic.h \
    src/base/ftbase.h \
    src/base/md5.h \
    src/bdf/bdf.h \
    src/bdf/bdfdrivr.h \
    src/bdf/bdferror.h \
    src/cache/ftccache.h \
    src/cache/ftccback.h \
    src/cache/ftcerror.h \
    src/cache/ftcglyph.h \
    src/cache/ftcimage.h \
    src/cache/ftcmanag.h \
    src/cache/ftcmru.h \
    src/cache/ftcsbits.h \
    src/cff/cf2arrst.h \
    src/cff/cf2blues.h \
    src/cff/cf2error.h \
    src/cff/cf2fixed.h \
    src/cff/cf2font.h \
    src/cff/cf2ft.h \
    src/cff/cf2glue.h \
    src/cff/cf2hints.h \
    src/cff/cf2intrp.h \
    src/cff/cf2read.h \
    src/cff/cf2stack.h \
    src/cff/cf2types.h \
    src/cff/cffcmap.h \
    src/cff/cffdrivr.h \
    src/cff/cfferrs.h \
    src/cff/cffgload.h \
    src/cff/cffload.h \
    src/cff/cffobjs.h \
    src/cff/cffparse.h \
    src/cff/cffpic.h \
    src/cff/cfftoken.h \
    src/cff/cfftypes.h \
    src/cid/ciderrs.h \
    src/cid/cidgload.h \
    src/cid/cidload.h \
    src/cid/cidobjs.h \
    src/cid/cidparse.h \
    src/cid/cidriver.h \
    src/cid/cidtoken.h \
    src/gxvalid/gxvalid.h \
    src/gxvalid/gxvcommn.h \
    src/gxvalid/gxverror.h \
    src/gxvalid/gxvfeat.h \
    src/gxvalid/gxvmod.h \
    src/gxvalid/gxvmort.h \
    src/gxvalid/gxvmorx.h \
    src/gzip/ftzconf.h \
    src/gzip/infblock.h \
    src/gzip/infcodes.h \
    src/gzip/inffixed.h \
    src/gzip/inftrees.h \
    src/gzip/infutil.h \
    src/gzip/zlib.h \
    src/gzip/zutil.h \
    src/lzw/ftzopen.h \
    src/otvalid/otvalid.h \
    src/otvalid/otvcommn.h \
    src/otvalid/otverror.h \
    src/otvalid/otvgpos.h \
    src/otvalid/otvmod.h \
    src/pcf/pcf.h \
    src/pcf/pcfdrivr.h \
    src/pcf/pcferror.h \
    src/pcf/pcfread.h \
    src/pcf/pcfutil.h \
    src/pfr/pfrcmap.h \
    src/pfr/pfrdrivr.h \
    src/pfr/pfrerror.h \
    src/pfr/pfrgload.h \
    src/pfr/pfrload.h \
    src/pfr/pfrobjs.h \
    src/pfr/pfrsbit.h \
    src/pfr/pfrtypes.h \
    src/psaux/afmparse.h \
    src/psaux/psauxerr.h \
    src/psaux/psauxmod.h \
    src/psaux/psconv.h \
    src/psaux/psobjs.h \
    src/psaux/t1cmap.h \
    src/psaux/t1decode.h \
    src/pshinter/pshalgo.h \
    src/pshinter/pshglob.h \
    src/pshinter/pshmod.h \
    src/pshinter/pshnterr.h \
    src/pshinter/pshpic.h \
    src/pshinter/pshrec.h \
    src/psnames/psmodule.h \
    src/psnames/psnamerr.h \
    src/psnames/pspic.h \
    src/psnames/pstables.h \
    src/raster/ftmisc.h \
    src/raster/ftraster.h \
    src/raster/ftrend1.h \
    src/raster/rasterrs.h \
    src/raster/rastpic.h \
    src/sfnt/pngshim.h \
    src/sfnt/sfdriver.h \
    src/sfnt/sferrors.h \
    src/sfnt/sfntpic.h \
    src/sfnt/sfobjs.h \
    src/sfnt/ttbdf.h \
    src/sfnt/ttcmap.h \
    src/sfnt/ttcmapc.h \
    src/sfnt/ttkern.h \
    src/sfnt/ttload.h \
    src/sfnt/ttmtx.h \
    src/sfnt/ttpost.h \
    src/sfnt/ttsbit.h \
    src/smooth/ftgrays.h \
    src/smooth/ftsmerrs.h \
    src/smooth/ftsmooth.h \
    src/smooth/ftspic.h \
    src/truetype/ttdriver.h \
    src/truetype/tterrors.h \
    src/truetype/ttgload.h \
    src/truetype/ttgxvar.h \
    src/truetype/ttinterp.h \
    src/truetype/ttobjs.h \
    src/truetype/ttpic.h \
    src/truetype/ttpload.h \
    src/truetype/ttsubpix.h \
    src/type1/t1afm.h \
    src/type1/t1driver.h \
    src/type1/t1errors.h \
    src/type1/t1gload.h \
    src/type1/t1load.h \
    src/type1/t1objs.h \
    src/type1/t1parse.h \
    src/type1/t1tokens.h \
    src/type42/t42drivr.h \
    src/type42/t42error.h \
    src/type42/t42objs.h \
    src/type42/t42parse.h \
    src/type42/t42types.h \
    src/winfonts/fnterrs.h \
    src/winfonts/winfnt.h \
    include/freetype/config/ftconfig.h \
    include/freetype/config/ftheader.h \
    include/freetype/config/ftmodule.h \
    include/freetype/config/ftoption.h \
    include/freetype/config/ftstdlib.h \
    include/freetype/internal/services/svbdf.h \
    include/freetype/internal/services/svcid.h \
    include/freetype/internal/services/svfntfmt.h \
    include/freetype/internal/services/svgldict.h \
    include/freetype/internal/services/svgxval.h \
    include/freetype/internal/services/svkern.h \
    include/freetype/internal/services/svmetric.h \
    include/freetype/internal/services/svmm.h \
    include/freetype/internal/services/svotval.h \
    include/freetype/internal/services/svpfr.h \
    include/freetype/internal/services/svpostnm.h \
    include/freetype/internal/services/svprop.h \
    include/freetype/internal/services/svpscmap.h \
    include/freetype/internal/services/svpsinfo.h \
    include/freetype/internal/services/svsfnt.h \
    include/freetype/internal/services/svttcmap.h \
    include/freetype/internal/services/svtteng.h \
    include/freetype/internal/services/svttglyf.h \
    include/freetype/internal/services/svwinfnt.h \
    include/freetype/internal/autohint.h \
    include/freetype/internal/ftcalc.h \
    include/freetype/internal/ftdebug.h \
    include/freetype/internal/ftdriver.h \
    include/freetype/internal/ftgloadr.h \
    include/freetype/internal/fthash.h \
    include/freetype/internal/ftmemory.h \
    include/freetype/internal/ftobjs.h \
    include/freetype/internal/ftpic.h \
    include/freetype/internal/ftrfork.h \
    include/freetype/internal/ftserv.h \
    include/freetype/internal/ftstream.h \
    include/freetype/internal/fttrace.h \
    include/freetype/internal/ftvalid.h \
    include/freetype/internal/internal.h \
    include/freetype/internal/psaux.h \
    include/freetype/internal/pshints.h \
    include/freetype/internal/sfnt.h \
    include/freetype/internal/t1types.h \
    include/freetype/internal/tttypes.h \
    include/freetype/freetype.h \
    include/freetype/ftadvanc.h \
    include/freetype/ftautoh.h \
    include/freetype/ftbbox.h \
    include/freetype/ftbdf.h \
    include/freetype/ftbitmap.h \
    include/freetype/ftbzip2.h \
    include/freetype/ftcache.h \
    include/freetype/ftcffdrv.h \
    include/freetype/ftchapters.h \
    include/freetype/ftcid.h \
    include/freetype/fterrdef.h \
    include/freetype/fterrors.h \
    include/freetype/ftfntfmt.h \
    include/freetype/ftgasp.h \
    include/freetype/ftglyph.h \
    include/freetype/ftgxval.h \
    include/freetype/ftgzip.h \
    include/freetype/ftimage.h \
    include/freetype/ftincrem.h \
    include/freetype/ftlcdfil.h \
    include/freetype/ftlist.h \
    include/freetype/ftlzw.h \
    include/freetype/ftmac.h \
    include/freetype/ftmm.h \
    include/freetype/ftmodapi.h \
    include/freetype/ftmoderr.h \
    include/freetype/ftotval.h \
    include/freetype/ftoutln.h \
    include/freetype/ftpcfdrv.h \
    include/freetype/ftpfr.h \
    include/freetype/ftrender.h \
    include/freetype/ftsizes.h \
    include/freetype/ftsnames.h \
    include/freetype/ftstroke.h \
    include/freetype/ftsynth.h \
    include/freetype/ftsystem.h \
    include/freetype/fttrigon.h \
    include/freetype/ftttdrv.h \
    include/freetype/fttypes.h \
    include/freetype/ftwinfnt.h \
    include/freetype/t1tables.h \
    include/freetype/ttnameid.h \
    include/freetype/tttables.h \
    include/freetype/tttags.h \
    include/freetype/ttunpat.h \
    include/ft2build.h \
    opengl_includes.hpp

DISTFILES += \
    data/font/arial.ttf \
    data/font/dahot_Garfield_www_myfontfree_com.ttf \
    data/font/design_graffiti_agentorange_www_myfontfree_com.ttf

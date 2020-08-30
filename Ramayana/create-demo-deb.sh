#!/bin/sh

TEMPDIR=~/ramayana-demo

clear

if [ -e $TEMPDIR ] 
then
	echo "cleaning up previous build"
	rm -r $TEMPDIR
fi

echo "creating file structure"
mkdir $TEMPDIR
mkdir $TEMPDIR/DEBIAN
mkdir $TEMPDIR/usr
mkdir $TEMPDIR/usr/local
mkdir $TEMPDIR/usr/local/bin
mkdir $TEMPDIR/usr/share
mkdir $TEMPDIR/usr/share/ramayana

echo "writing control file"
echo "Package: ramayana-rts-game-demo
Version: 0.0.0
Architecture: amd64
Installed-Size: 600000
Maintainer: Rakesh Malik <rakeshmalik91@gmail.com>
Depends: libgles2-mesa (>=10.1.0), libglu1-mesa (>=9.0.0-2), libglew1.10 (>=1.10.0-3), freeglut3 (>=2.8.1-1), libsdl1.2debian (>=1.2.15-8), libsdl-mixer1.2 (>=1.2.12-10), libopencv-core2.4 (>=2.4.8), libopencv-imgproc2.4 (>=2.4.8), libopencv-video2.4 (>=2.4.8), libopencv-highgui2.4 (>=2.4.8)
Description: A real time strategy game on Indian epic Ramayana.
Section: games
Priority: optional" >> $TEMPDIR/DEBIAN/control

echo "creating link"
echo "#!/bin/sh
cd /usr/share/ramayana
./Ramayana &
" >> $TEMPDIR/usr/local/bin/ramayana
chmod 755 $TEMPDIR/usr/local/bin/ramayana

echo "copying"
echo " --executable"
cp -r Demo\ Release/Ramayana $TEMPDIR/usr/share/ramayana/Ramayana
echo " --icons"
cp -r Ramayana.ico $TEMPDIR/usr/share/ramayana/
cp -r Ramayana_small.ico $TEMPDIR/usr/share/ramayana/
echo " --audio"
cp -r audio $TEMPDIR/usr/share/ramayana/
echo " --cursor"
cp -r cursor $TEMPDIR/usr/share/ramayana/
echo " --kb"
cp -r kb $TEMPDIR/usr/share/ramayana/
echo " --shaders"
cp -r shaders $TEMPDIR/usr/share/ramayana/
echo " --special"
cp -r special $TEMPDIR/usr/share/ramayana/
echo " --map"
mkdir $TEMPDIR/usr/share/ramayana/map
echo " --save, log"
mkdir $TEMPDIR/usr/share/ramayana/save
mkdir $TEMPDIR/usr/share/ramayana/log

echo " --campaign"
mkdir $TEMPDIR/usr/share/ramayana/campaign
cp campaign/01* $TEMPDIR/usr/share/ramayana/campaign/

echo " --data"
mkdir $TEMPDIR/usr/share/ramayana/data
cp data/unit_deploy_demo.xml $TEMPDIR/usr/share/ramayana/data/unit.xml
cp data/settings_deploy.xml $TEMPDIR/usr/share/ramayana/data/settings.xml
cp data/campaign_deploy.xml $TEMPDIR/usr/share/ramayana/data/campaign.xml

echo " --ui"
mkdir $TEMPDIR/usr/share/ramayana/ui
cp ui/*.png $TEMPDIR/usr/share/ramayana/ui/
mkdir $TEMPDIR/usr/share/ramayana/ui/campaign_icon
cp ui/campaign_icon/1.png $TEMPDIR/usr/share/ramayana/ui/campaign_icon/
cp ui/campaign_icon/2.png $TEMPDIR/usr/share/ramayana/ui/campaign_icon/
mkdir $TEMPDIR/usr/share/ramayana/ui/campaign_image
cp ui/campaign_image/1.png $TEMPDIR/usr/share/ramayana/ui/campaign_image/
mkdir $TEMPDIR/usr/share/ramayana/ui/campaign_name
cp ui/campaign_name/1.png $TEMPDIR/usr/share/ramayana/ui/campaign_name/

echo " --unit"
mkdir $TEMPDIR/usr/share/ramayana/unit
cp -r unit/asvattha $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/bald_tree $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/bamboo $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/boar $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/crow $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/cycus $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/cycus2 $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/date $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/deer $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/eagle $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/fern $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/fern2 $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/flag_long $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/flamboyant $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/grass $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/grey_stone0 $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/hut $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/ivy $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/jackfruit $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/jatayu $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/kabandha $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/lakshmana $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/lanka_swordsman $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/mango $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/mine $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/neem $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/sita_evidance $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/tamarind $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/traveler $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/tulip $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/villager_female $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/villager_female_2 $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/villager_male $TEMPDIR/usr/share/ramayana/unit/
cp -r unit/villager_male_2 $TEMPDIR/usr/share/ramayana/unit/
mkdir $TEMPDIR/usr/share/ramayana/unit/rama
cp unit/rama/*.png $TEMPDIR/usr/share/ramayana/unit/rama/
cp unit/rama/*.jpg $TEMPDIR/usr/share/ramayana/unit/rama/
cp unit/rama/base.unit $TEMPDIR/usr/share/ramayana/unit/rama/
cp unit/rama/sword.unit $TEMPDIR/usr/share/ramayana/unit/rama/

echo " --video"
mkdir $TEMPDIR/usr/share/ramayana/video
cp -r video/loading_game $TEMPDIR/usr/share/ramayana/video/
cp -r video/menu $TEMPDIR/usr/share/ramayana/video/
mkdir $TEMPDIR/usr/share/ramayana/video/loading_map
mkdir $TEMPDIR/usr/share/ramayana/video/loading_map/480p
cp -r video/loading_map/480p/gandhamadan.avi $TEMPDIR/usr/share/ramayana/video/loading_map/480p/
cp -r video/loading_map/480p/rama_sword.avi $TEMPDIR/usr/share/ramayana/video/loading_map/480p/
mkdir $TEMPDIR/usr/share/ramayana/video/loading_map/720p
cp -r video/loading_map/720p/gandhamadan.avi $TEMPDIR/usr/share/ramayana/video/loading_map/720p/
cp -r video/loading_map/720p/rama_sword.avi $TEMPDIR/usr/share/ramayana/video/loading_map/720p/
mkdir $TEMPDIR/usr/share/ramayana/video/loading_map/1080p
cp -r video/loading_map/1080p/gandhamadan.avi $TEMPDIR/usr/share/ramayana/video/loading_map/1080p/
cp -r video/loading_map/1080p/rama_sword.avi $TEMPDIR/usr/share/ramayana/video/loading_map/1080p/
mkdir $TEMPDIR/usr/share/ramayana/video/cutscene
mkdir $TEMPDIR/usr/share/ramayana/video/cutscene/480p
cp -r video/cutscene/480p/cmpn01_* $TEMPDIR/usr/share/ramayana/video/cutscene/480p/
mkdir $TEMPDIR/usr/share/ramayana/video/cutscene/720p
cp -r video/cutscene/720p/cmpn01_* $TEMPDIR/usr/share/ramayana/video/cutscene/720p/
mkdir $TEMPDIR/usr/share/ramayana/video/cutscene/1080p
cp -r video/cutscene/1080p/cmpn01_* $TEMPDIR/usr/share/ramayana/video/cutscene/1080p/
mkdir $TEMPDIR/usr/share/ramayana/video/cutscene/audio_track
cp -r video/cutscene/audio_track/cmpn01_* $TEMPDIR/usr/share/ramayana/video/cutscene/audio_track/

echo "setting file permission"
chmod -R 775 $TEMPDIR

echo "creating deb package"
dpkg-deb -b $TEMPDIR

echo "moving package here"
mv $TEMPDIR.deb distributable/

echo "cleaning up"
rm -r $TEMPDIR

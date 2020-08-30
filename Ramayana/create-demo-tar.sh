#!/bin/sh

TEMPDIR=~/ramayana-demo

clear

if [ -e $TEMPDIR ] 
then
	echo "cleaning up previous build"
	rm -r $TEMPDIR
fi

echo "copying"
mkdir $TEMPDIR
echo " --executable"
cp -r Demo\ Release/Ramayana $TEMPDIR/Ramayana
echo " --icons"
cp -r Ramayana.ico $TEMPDIR/
cp -r Ramayana_small.ico $TEMPDIR/
echo " --audio"
cp -r audio $TEMPDIR/
echo " --cursor"
cp -r cursor $TEMPDIR/
echo " --kb"
cp -r kb $TEMPDIR/
echo " --shaders"
cp -r shaders $TEMPDIR/
echo " --special"
cp -r special $TEMPDIR/
echo " --map"
mkdir $TEMPDIR/map
echo " --save, log"
mkdir $TEMPDIR/save
mkdir $TEMPDIR/log

echo " --campaign"
mkdir $TEMPDIR/campaign
cp campaign/01* $TEMPDIR/campaign/

echo " --data"
mkdir $TEMPDIR/data
cp data/unit_deploy_demo.xml $TEMPDIR/data/unit.xml
cp data/settings_deploy.xml $TEMPDIR/data/settings.xml
cp data/campaign_deploy.xml $TEMPDIR/data/campaign.xml

echo " --ui"
mkdir $TEMPDIR/ui
cp ui/*.png $TEMPDIR/ui/
mkdir $TEMPDIR/ui/campaign_icon
cp ui/campaign_icon/1.png $TEMPDIR/ui/campaign_icon/
cp ui/campaign_icon/2.png $TEMPDIR/ui/campaign_icon/
mkdir $TEMPDIR/ui/campaign_image
cp ui/campaign_image/1.png $TEMPDIR/ui/campaign_image/
mkdir $TEMPDIR/ui/campaign_name
cp ui/campaign_name/1.png $TEMPDIR/ui/campaign_name/

echo " --unit"
mkdir $TEMPDIR/unit
cp -r unit/asvattha $TEMPDIR/unit/
cp -r unit/bald_tree $TEMPDIR/unit/
cp -r unit/bamboo $TEMPDIR/unit/
cp -r unit/boar $TEMPDIR/unit/
cp -r unit/crow $TEMPDIR/unit/
cp -r unit/cycus $TEMPDIR/unit/
cp -r unit/cycus2 $TEMPDIR/unit/
cp -r unit/date $TEMPDIR/unit/
cp -r unit/deer $TEMPDIR/unit/
cp -r unit/eagle $TEMPDIR/unit/
cp -r unit/fern $TEMPDIR/unit/
cp -r unit/fern2 $TEMPDIR/unit/
cp -r unit/flag_long $TEMPDIR/unit/
cp -r unit/flamboyant $TEMPDIR/unit/
cp -r unit/grass $TEMPDIR/unit/
cp -r unit/grey_stone0 $TEMPDIR/unit/
cp -r unit/hut $TEMPDIR/unit/
cp -r unit/ivy $TEMPDIR/unit/
cp -r unit/jackfruit $TEMPDIR/unit/
cp -r unit/jatayu $TEMPDIR/unit/
cp -r unit/kabandha $TEMPDIR/unit/
cp -r unit/lakshmana $TEMPDIR/unit/
cp -r unit/lanka_swordsman $TEMPDIR/unit/
cp -r unit/mango $TEMPDIR/unit/
cp -r unit/mine $TEMPDIR/unit/
cp -r unit/neem $TEMPDIR/unit/
cp -r unit/sita_evidance $TEMPDIR/unit/
cp -r unit/tamarind $TEMPDIR/unit/
cp -r unit/traveler $TEMPDIR/unit/
cp -r unit/tulip $TEMPDIR/unit/
cp -r unit/villager_female $TEMPDIR/unit/
cp -r unit/villager_female_2 $TEMPDIR/unit/
cp -r unit/villager_male $TEMPDIR/unit/
cp -r unit/villager_male_2 $TEMPDIR/unit/
mkdir $TEMPDIR/unit/rama
cp unit/rama/*.png $TEMPDIR/unit/rama/
cp unit/rama/*.jpg $TEMPDIR/unit/rama/
cp unit/rama/base.unit $TEMPDIR/unit/rama/
cp unit/rama/sword.unit $TEMPDIR/unit/rama/

echo " --video"
mkdir $TEMPDIR/video
cp -r video/loading_game $TEMPDIR/video/
cp -r video/menu $TEMPDIR/video/
mkdir $TEMPDIR/video/loading_map
mkdir $TEMPDIR/video/loading_map/480p
cp -r video/loading_map/480p/gandhamadan.avi $TEMPDIR/video/loading_map/480p/
cp -r video/loading_map/480p/rama_sword.avi $TEMPDIR/video/loading_map/480p/
mkdir $TEMPDIR/video/loading_map/720p
cp -r video/loading_map/720p/gandhamadan.avi $TEMPDIR/video/loading_map/720p/
cp -r video/loading_map/720p/rama_sword.avi $TEMPDIR/video/loading_map/720p/
mkdir $TEMPDIR/video/loading_map/1080p
cp -r video/loading_map/1080p/gandhamadan.avi $TEMPDIR/video/loading_map/1080p/
cp -r video/loading_map/1080p/rama_sword.avi $TEMPDIR/video/loading_map/1080p/
mkdir $TEMPDIR/video/cutscene
mkdir $TEMPDIR/video/cutscene/480p
cp -r video/cutscene/480p/cmpn01_* $TEMPDIR/video/cutscene/480p/
mkdir $TEMPDIR/video/cutscene/720p
cp -r video/cutscene/720p/cmpn01_* $TEMPDIR/video/cutscene/720p/
mkdir $TEMPDIR/video/cutscene/1080p
cp -r video/cutscene/1080p/cmpn01_* $TEMPDIR/video/cutscene/1080p/
mkdir $TEMPDIR/video/cutscene/audio_track
cp -r video/cutscene/audio_track/cmpn01_* $TEMPDIR/video/cutscene/audio_track/

echo "setting file permission"
chmod -R 777 $TEMPDIR

echo "creating tarball"
tar -zcvf distributable/ramayana-demo.tar.gz $TEMPDIR

echo "cleaning up"
rm -r $TEMPDIR

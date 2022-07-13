echo "GD32 works with ST-LINK v2 but does not work with ST-LINK v3"
st-link_cli -c SWD -P build\wideband.hex -Rst -Run

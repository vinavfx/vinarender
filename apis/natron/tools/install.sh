natron_core='/usr/share/Natron/Plugins'
natron_plugins='/opt/Natron2/Plugins/PyPlugs'

plugins='./plugins'
core='./core'

mkdir -p $plugins

# copia plugins de python a natron
cp '../util.py' $natron_plugins
cp $plugins'/vvtext.py' $natron_plugins
cp $plugins'/vvtext_core.py' $natron_plugins

# copia el init a natron
cp $core'/init.py' $natron_core
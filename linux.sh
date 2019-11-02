#!/usr/bin/env sh
path="$( cd "$(dirname "$0")" ; pwd -P )"

# ruta de instalacion
dst="/opt/vinarender"
# ------------------
# IP del manager
ip="192.168.1.40"
# ------------------
manager_start=true
server_start=true


compile() {
    folder=$path/src/$1
    cd $folder
    qmake-qt5
    make
    bin=$path/bin
    mkdir -p $bin
    mv $folder/$2 $bin 
}

install() {
    # Instalacion de Dependencias
    yum -y install epel-release http://li.nux.ro/download/nux/dextop/el7/x86_64/nux-dextop-release-0-5.el7.nux.noarch.rpm
    yum -y install \
    qt5-qtbase \
    qt5-qtbase-devel \
    qt5-qtmultimedia.x86_64 \
    qt5-qtmultimedia-devel.x86_64 \
    qt5-qtsvg.x86_64 \
    qt5-qtsvg-devel.x86_64 \
    mesa-libGL-devel \
    mesa-libGLU-devel \
    pulseaudio-libs-glib2 \
    ffmpeg \
    lm_sensors \
    gcc-c++ \
    sshpass \
    psmisc #fuser

    yum -y group install "Development Tools"
    # ----------------------

    # Compilacion de todo
    compile server vserver
    compile manager vmanager
    compile monitor vmonitor
    compile submit submit 
    compile videovina videovina 
    # ----------------------

    echo $ip > $path"/etc/manager_host"

    # Creacion de servicios
    cp $path/os/linux/init/vserver.service /etc/systemd/system
    cp $path/os/linux/init/vmanager.service /etc/systemd/system
    systemctl daemon-reload
    # -----------------------------

    # los servicios son muy estrictos asi esto corrige el servicio si de modifico mal
    sed -i -e 's/\r//g' $path/os/linux/init/vserver.sh
    sed -i -e 's/\r//g' $path/os/linux/init/vmanager.sh
    # --------------------------------------------------------------------------------

    mkdir $dst
    mkdir $dst/os
    mkdir $dst/bin

    # # copia el contenido necesario
    cp -rf "$path/bin" $dst
    cp -rf "$path/etc" $dst
    cp -rf "$path/icons" $dst
    cp -rf "$path/log" $dst
    cp -rf "$path/os/linux" $dst/os
    cp -rf "$path/sound" $dst
    cp -rf "$path/modules" $dst
    cp -rf "$path/theme" $dst
    # # ------------------------

    chmod 755 -R $dst

    # inicializacion de servicios
    if $server_start; then
        systemctl start vserver
        systemctl enable vserver
    fi

    if $manager_start; then
        systemctl start vmanager
        systemctl enable vmanager
    fi
    # -----------------------------

    # Acceso directo
    echo "[Desktop Entry]
    Name=VinaRender Monitor
    Exec=/opt/vinarender/bin/vmonitor
    Icon=/opt/vinarender/icons/monitor.png
    Categories=Graphics;2DGraphics;RasterGraphics;FLTK;
    Type=Application" > "/usr/share/applications/VinaRender.desktop"
    # ---------------
}

uninstall() {
    systemctl stop vmanager
    systemctl stop vserver

    rm /etc/systemd/system/vmanager.service
    rm /etc/systemd/system/vserver.service

    rm "/usr/share/applications/VinaRender.desktop"
    rm -rf $dst
}

if [ $1 == install ]; then
    install
fi
if [ $1 == uninstall ]; then
    uninstall
fi
if [ $1 == reinstall ]; then
    uninstall
    install
fi
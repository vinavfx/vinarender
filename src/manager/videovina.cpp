#include "manager.h"

QString manager::videovina(QJsonArray recv)
{
    QString slideshow = assets + "/slideshows";
    QString music_dir = assets + "/music";

    // Datos recividos del servidor principal
    QString user = recv[0].toString();
    QString user_id = recv[1].toString();
    QString project_type = recv[2].toString();
    QString project_name = recv[3].toString();
    bool proxy = recv[4].toString().toInt();
    // -------------------------------------

    // Abre el project json del el respaldo de as3
    QString vv_project_dir = as3 + "/private/" + user + "/projects/" + project_name;
    QString vv_project_dir_public = as3 + "/public/" + user_id + "/projects/" + project_name;
    QString vv_project_json = vv_project_dir + "/project.json";
    QJsonObject vv_project = jread(vv_project_json);
    // --------------------------------------------

    // Directorios locales
    QString user_dir = vv_local_folder + "/" + user;
    QString project_dir = user_dir + "/" + project_name;
    QString project;
    if (proxy)
        project = project_dir + "/" + project_type + "_proxy.aep";
    else
        project = project_dir + "/" + project_type + ".aep";
    // ---------------------------------

    print("copiando footage...");

    // Cambia la ruta de la musica y la guarda el el project.json
    QString song = vv_project["song"].toString() + ".mp3";
    vv_project["songPath"] = music_dir + "/" + song;
    // -----------------------------------

    // guarda ruta de los proyectos slideshows
    vv_project["slideshowPath"] = slideshow;
    // -----------------------------------
    vv_project["proxy"] = proxy;

    // guarda el user_id en el proyecto
    vv_project["user_id"] = user_id;
    // ----------------

    os::makedirs(project_dir + "/renders");

    // Copia la plantilla del proyecto en el directorio local
    QString original_project;
    if (proxy)
        original_project = slideshow + "/" + project_type + "/" + project_type + "_proxy.aep";
    else
        original_project = slideshow + "/" + project_type + "/" + project_type + ".aep";
    os::copy(original_project, project);
    // -------------------------------------------

    // Copia footage, project.json de amazon S3 y lo copia en el directorio compartido local
    os::copydir(vv_project_dir + "/footage", project_dir);

    QString copy_proxy_footage = "cp " + vv_project_dir_public + "/footage/* " + project_dir + "/footage";
    os::system(copy_proxy_footage);

    jwrite(project_dir + "/project.json", vv_project);
    // -------------------------------------------
   
    // Permisos de carpeta de proyecto
    os::sh("chmod 777 -R " + project_dir);
    // -------------------------------------

    // Modifica el proyecto de after effect con los datos del proyecto del usuario
    QString vina2ae = catsfarm + "/modules/videovina/vina2ae.jsx";
    QString afterfx = "/opt/AE9.0/AfterFX.exe";
    QString cmd = "wine " + afterfx + " -noui -s \"var aep = '" + project + "';//@include '" + vina2ae + "';\"";
    print("start project");
    os::sh(cmd);
    print("end.");
    // ---------------------------------------------------

    // genera la ruta base de la salida del mov, no es completa por que despues en el render
    // hay que ponerle el numero del video y la extencion por cada tarea para poder hacer el concat
    QString output = project_dir + "/renders/" + project_name + "/" + project_name;
    // ---------------------------------------------------

    // Lee los datos de salida del script vina2ae.jsx
    QString submit_json = project_dir + "/submit.json";
    QJsonObject submit = jread(submit_json); // aumento de ram cuando no encuetra el archivo json ( ¡revisar)
    // ---------------------------------------------

    // aveces en las pruebas cuando vina2ae.jsx da un error no crea el submit.json y no lo puede leer
    // por eso si no hay informacion retorna para que no de conflicto despues
    if (submit.empty())
        return "";
    // ------------------------------------

    QString job_name = user + "-" + project_name;
    QString server = "None";
    QString server_group = "videovina";
    int first_frame = 1;
    int last_frame = submit["last_frame"].toInt();
    int task_size = 50;
    int priority = 1;
    bool suspend = false;
    QString comment = "VideoVina Render";
    QString software = "AE";
    QString extra = output;
    QString _system = "linux";
    int instances;
    if (proxy)
        instances = 7;
    else
        instances = 3;
    QString render = "Final comp";

    QJsonArray data = {
        job_name,
        server,
        server_group,
        first_frame,
        last_frame,
        task_size,
        priority,
        suspend,
        comment,
        software,
        project,
        extra,
        _system,
        instances,
        render

    };

    make_job(data);

    return "";
}

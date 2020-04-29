#include "render.hpp"

render_class::render_class(QMutex *_mutex)
{
	mutex = _mutex;
	// inicializar instancias 16 veces
	for (int i = 0; i < 15; ++i)
	{
		first_frame.push_back(0);
		last_frame.push_back(0);
		pid.push_back(0);

		taskKill.push_back(false);
		renderInstance.push_back(false);

		project.push_back("none");
		jobSystem.push_back("none");
		extra.push_back("none");
		renderNode.push_back("none");
		vmSoftware.push_back("none");
		src_path.push_back("none");
		dst_path.push_back("none");
	} //-------------------------------------------

	threading(&render_class::suspend_vbox, this);
}

QString render_class::render_task(QJsonArray recv)
{
	// comvierte lista de json en variables usables
	QString status;

	mutex->lock();
	int ins = recv[2].toInt();
	QString software = recv[1].toString();
	project[ins] = recv[0].toString();
	first_frame[ins] = recv[3].toInt();
	last_frame[ins] = recv[4].toInt();
	jobSystem[ins] = recv[5].toString();
	extra[ins] = recv[6].toString();
	renderNode[ins] = recv[7].toString();
	//------------------------------------------------------------

	// si alguna de las instancias ya esta en render no renderea
	bool renderNow = false;

	if (not renderInstance[ins])
	{
		renderInstance[ins] = true;
		renderNow = true;
	}
	//-----------------------------------------------------------------
	mutex->unlock();
	if (renderNow)
	{
		mutex->lock();
		//obtiene ruta correcta
		QJsonArray system_path = preferences["paths"].toObject()["system"].toArray();

		auto correctPath = find_correct_path(system_path, project[ins]);
		src_path[ins] = correctPath[0];
		dst_path[ins] = correctPath[1];
		//------------------------------------------------------
		mutex->unlock();

		// ---------- rendering softwares -----------------
		bool renderOK = false;
		if (software == "Nuke")
			renderOK = nuke(ins);
		if (software == "Houdini")
			renderOK = houdini(ins);
		if (software == "Maya")
			renderOK = maya(ins);
		if (software == "Natron")
			renderOK = natron(ins);
		if (software == "Cinema4D")
			renderOK = cinema(ins);
		if (software == "AE")
			renderOK = ae(ins);
		// -------------------------------------------------

		QString log_file = path + "/log/render_log_" + QString::number(ins);

		mutex->lock();
		if (taskKill[ins])
		{
			taskKill[ins] = false;
			status = "kill";
		}
		else
		{
			if (renderOK)
				status = "ok";
			else
			{
				status = "failed";
				os::copy(log_file, path + "/log/render_log");
			}
		}

		// Habilita las instancia que ya termino, para que pueda renderear
		renderInstance[ins] = false;
		//---------------------------------------------------------------------
		mutex->unlock();
	}

	return status;
}
QList<QString> render_class::find_correct_path(QJsonArray system_path, QString _path)
{
	//obtiene ruta correcta
	QString proj;
	QString src;
	QString dst;
	for (QJsonValue p1 : system_path)
	{
		for (QJsonValue p2 : system_path)
		{
			proj = _path;
			proj.replace(p1.toString(), p2.toString());

			if (os::isfile(proj) || os::isdir(proj))
			{
				src = p1.toString();
				dst = p2.toString();
				break;
			}
		}

		if (os::isfile(proj) || os::isdir(proj))
			break;
	}

	return {src, dst};
}

QString render_class::qprocess(QString cmd, int ins)
{
	QProcess proc;
	proc.start(cmd);
	if (ins > -1)
		pid[ins] = proc.processId();
	proc.waitForFinished(-1);
	QString output = proc.readAllStandardOutput() + "\n" + proc.readAllStandardError();
	proc.close();

	return output;
}

void render_class::vbox_turn(bool turn)
{

	QString vm;
	if (turn)
	{
		if (_linux)
			vm = "VBoxManage startvm win2016 --type headless";
		if (_win32)
			vm = "\"C:/Program Files/Oracle/VirtualBox/VBoxManage.exe\" startvm win2016 --type headless";

		os::back(vm);
	}
	else
	{
		if (_linux)
			vm = "VBoxManage controlvm win2016 savestate";
		if (_win32)
			vm = "\"C:/Program Files/Oracle/VirtualBox/VBoxManage.exe\" controlvm win2016 savestate";

		os::back(vm);

		fwrite(path + "/log/vbox", "0");
	}
}

bool render_class::vbox_working()
{
	// Virtual Machin status

	QString vm;
	if (_linux)
		vm = "VBoxManage list runningvms";

	if (_win32)
		vm = "\"C:/Program Files/Oracle/VirtualBox/VBoxManage.exe\" list runningvms";

	QString running = os::sh(vm).split(" ")[0];

	if (running == "\"win2016\"")
	{
		return true;
	}
	else
	{
		return false;
	}
	//------------------------------------------
}

void render_class::suspend_vbox()
{

	// checkea si el Cinama 4D esta en render y si no se apaga la maquina virtual
	VMCinemaRunningTimes = 0;
	while (1)
	{
		if (not VMCinemaActive)
		{
			VMCinemaRunningTimes++;
			if (VMCinemaRunningTimes > 10)
			{ // si no se esta usando Cinema4D, al numero 10 se apaga la maquina
				if (vbox_working() and VMCinemaTurn)
				{ // solo si esta prendida y si la prendio vinarender
					vbox_turn(false);
					VMCinemaTurn = false;
				}
				VMCinemaRunningTimes = 0;
			}
		}
		sleep(1);
	} //-----------------------------------------------------------------------
}

bool render_class::nuke(int ins)
{
	mutex->lock();
	// Ecuentra ruta del write y la remplaza por la ruta correcta segun OS
	QJsonArray system_path = preferences["paths"].toObject()["system"].toArray();
	auto correctPath = find_correct_path(system_path, os::dirname(extra[ins]));

	QString proj = project[ins];
	proj.replace(src_path[ins], dst_path[ins]);

	QString tmpProj = proj;
	tmpProj.replace(".nk", "_" + os::hostName() + ".nk");

	QString nk = fread(proj);
	nk.replace(correctPath[0], correctPath[1]);

	fwrite(tmpProj, nk);
	// ------------------------------------------------------

	// crea la carpeta donde se renderearan los archivos
	QString dirFile = extra[ins].replace(correctPath[0], correctPath[1]);
	mutex->unlock();

	QString dirRender = os::dirname(dirFile);
	QString fileRender = os::basename(dirFile);

	QString ext = fileRender.split(".").last();
	fileRender = fileRender.split(".")[0];
	QString pad = fileRender.split("_").last();

	QString folder_name;
	if (ext == "mov")
		folder_name = fileRender;
	else
		folder_name = fileRender.replace("_" + pad, "");

	QString folderRender = dirRender + "/" + folder_name;

	if (not os::isdir(folderRender))
	{
		os::mkdir(folderRender);
		if (_linux)
			os::system("chmod 777 -R " + folderRender);
	}
	//---------------------------------------------------
	// Si es hay licencia de nodo de render nuke_r poner true
	bool nuke_r = false;
	QString xi = "";
	if (not nuke_r)
		xi = "-xi";
	// ----------------------------------
	mutex->lock();
	QString args = "-f " + xi + " -X " + renderNode[ins] + " \"" + tmpProj + "\" " + QString::number(first_frame[ins]) + "-" + QString::number(last_frame[ins]);
	// remapeo rutas de Nuke
	QString nukeRemap = " -remap \"" + src_path[ins] + "," + dst_path[ins] + "\" ";
	args = args.replace(src_path[ins], dst_path[ins]);
	args = nukeRemap + args;
	//------------------------------------------------

	//Obtiene el excecutable que existe en este sistema
	QString exe;
	for (auto e : preferences["paths"].toObject()["nuke"].toArray())
	{
		exe = e.toString();
		if (os::isfile(exe))
			break;
	}
	mutex->unlock();
	//-----------------------------------------------

	QString cmd = '"' + exe + '"' + args;

	QString log_file = path + "/log/render_log_" + QString::number(ins);

	// rendering ...
	// ----------------------------------
	QString log;
	log = qprocess(cmd, ins);
	// al hacer render en una comp nueva por primera vez
	// aparece un error de "missing close-brace" y para evitar que
	// llegue el error al manager intenta en un segundo si vuelve
	// a aparecer manda el error
	if (log.contains("missing close-brace"))
	{
		sleep(1);
		log = qprocess(cmd, ins);
	}
	// ---------------------------------------
	fwrite(log_file, log);
	// ----------------------------------

	// Borra proyecto temporal
	os::remove(tmpProj);
	// -----------------------------

	mutex->lock();
	int total_frame = last_frame[ins] - first_frame[ins] + 1;
	mutex->unlock();

	if (log.count("Frame ") == total_frame)
		return true;
	else
		return false;
}

bool render_class::maya(int ins)
{

	QString log_file = path + "/log/render_log_" + QString::number(ins);
	os::remove(log_file);

	QString args = " -r file -s " + QString::number(first_frame[ins]) + " -e " + QString::number(last_frame[ins]) +
				   " -proj '" + extra[ins] + "' '" + project[ins] + "'" + " -log '" + log_file + "'";

	//Obtiene el excecutable que existe en este sistema
	QString exe;

	for (auto e : preferences["paths"].toObject()["maya"].toArray())
	{
		exe = e.toString();
		if (os::isfile(exe))
		{
			break;
		}
	}
	//-----------------------------------------------
	args = args.replace(src_path[ins], dst_path[ins]);

	QString cmd = "/bin/sh -c \"export MAYA_DISABLE_CIP=1 && '" + exe + "' " + args + "\"";
	// rendering ...
	// ----------------------------------
	qprocess(cmd, ins);
	// ----------------------------------

	// post render
	QString log = fread(log_file);
	if (log.contains("completed."))
		return true;
	else
		return false;
	// --------------------------
}

bool render_class::cinema(int ins)
{

	QString log_file = path + "/log/render_log_" + QString::number(ins);
	QString log, cmd;

	if (_linux)
	{ // en linux se usa una maquina virtual
		VMCinemaActive = true;
		//Obtiene el excecutable de cinema de windows
		QString exe_windows;
		for (auto e : preferences["paths"].toObject()["cinema"].toArray())
		{
			exe_windows = e.toString();
			if (exe_windows.contains("C:/"))
				break;
		}
		//-----------------------------------------------

		// replaza las rutas para virtualbox
		for (auto s : preferences["paths"].toObject()["system"].toArray())
			project[ins] = project[ins].replace(s.toString(), "//VBOXSVR/server_01");
		//-------------------------------------------------------------------------

		// con este commando se puede enviar comandos a la maquina virtual y te regresa un resultado
		QString guestcontrol = "VBoxManage --nologo guestcontrol win2016 run --username Administrator --password Jump77cats --exe ";
		VMSH = guestcontrol + "C:\\\\Windows\\\\system32\\\\cmd.exe -- C:\\\\Windows\\\\SysWOW64\\\\cmd.exe \"/c\" ";
		// --------------------------------------------------------------

		QString args = "\"-frame\" \"" + QString::number(first_frame[ins]) + "\" \"" + QString::number(last_frame[ins]) + "\" \"-nogui\" \"-render\" \"" + project[ins] + "\"";
		cmd = guestcontrol + "\"" + exe_windows + "\" -- 0 " + args; // 0 es el primer argumento que seria el excecutable que esta en -exe

		// inicia virtual machine
		if (not vbox_working())
		{
			vbox_turn(true);
			VMCinemaTurn = true;
		}

		// checkea si la maquina esta lista para renderear
		QString check = VMSH + "\"echo vm_is_ready\"";

		int i = 0;
		bool problem = false;
		while (not qprocess(check).contains("vm_is_ready"))
		{ // espera que la maquina este lista
			sleep(1);
			if (i > 200)
			{
				problem = true;
				log = "The Virtual Machine has a problem.";
				break;
			}
		}
		//--------------------------------

		// rendering ...
		// ----------------------------------
		if (not problem)
			log = qprocess(cmd, ins);
		//-------------------------------
		fwrite(log_file, log);

		//para que se apague la maquina virtual si no se uas
		VMCinemaActive = false;
		VMCinemaRunningTimes = 0;
		//----------------------------
	}

	else
	{

		project[ins] = project[ins].replace(src_path[ins], dst_path[ins]);

		QString args = " -nogui -render \"" + project[ins] + "\" g_logfile=\"" + log_file + "\"" +
					   " -frame " + QString::number(first_frame[ins]) + " " + QString::number(last_frame[ins]);

		//Obtiene el excecutable que existe en este sistema
		QString exe;
		for (auto e : preferences["paths"].toObject()["cinema"].toObject())
		{
			exe = e.toString();
			if (os::isfile(exe))
			{
				break;
			}
		}
		//-----------------------------------------------

		cmd = '"' + exe + '"' + args;

		// rendering ...
		// ----------------------------------
		qprocess(cmd, ins);
		log = fread(log_file);
		//----------------------
	}

	// post render
	if (log.contains("Rendering successful: "))
		return true;
	else
		return false;
	//---------------------------
}

bool render_class::houdini(int ins)
{

	//Obtiene el excecutable que existe en este sistema
	QString exe;
	for (auto e : preferences["paths"].toObject()["houdini"].toArray())
	{
		exe = e.toString();
		if (os::isfile(exe))
		{
			break;
		}
	}
	//-----------------------------------------------

	QString hipFile = project[ins].replace(src_path[ins], dst_path[ins]);

	QString render_file = path + "/modules/houdiniVinaRender.py " +
						  hipFile + " " + renderNode[ins] + " " + QString::number(first_frame[ins]) + " " + QString::number(last_frame[ins]);

	QString cmd = '"' + exe + "\" " + render_file;

	QString log_file = path + "/log/render_log_" + QString::number(ins);

	// rendering ...
	// ----------------------------------
	QString log = qprocess(cmd, ins);
	fwrite(log_file, log);
	// ----------------------------------

	// post render
	int total_frame = last_frame[ins] - first_frame[ins] + 1;

	if (log.count(" frame ") == total_frame)
		return true;
	else
		return false;
	// ----------------------------------
}

bool render_class::natron(int ins)
{
	mutex->lock();

	//Obtiene el excecutable que existe en este sistema
	QString exe;
	for (auto e : preferences["paths"].toObject()["natron"].toArray())
	{
		exe = e.toString();
		if (os::isfile(exe))
			break;
	}
	//-----------------------------------------------
	QString firstFrame = QString::number(first_frame[ins]);
	QString lastFrame = QString::number(last_frame[ins]);

	QString natron_module = path + "/modules/natron/render.sh";

	// crea la carpeta donde se renderearan los archivos
	QString output_file = extra[ins];

	QString output_dir = os::dirname(output_file);
	QString output_name = os::basename(output_file);

	QString ext = output_name.split(".").last();

	QString output_render;
	QString output;
	if (ext == "mov")
	{
		output_name = output_name.split(".")[0];
		output_render = output_dir + "/" + output_name;

		// crea numero con ceros para el nombre a partir del primer cuadro
		QString num = "0000000000" + firstFrame;
		QStringRef nameNumber(&num, num.length() - 10, 10);
		// -------------------------------------

		output = output_render + "/" + output_name + "_" + nameNumber + ".mov";
	}
	else
	{
		output_render = output_dir + "/" + output_name.split("_")[0];
		output = output_render + "/" + output_name;
	}

	if (not os::isdir(output_render))
	{
		os::mkdir(output_render);
		if (_linux)
			os::system("chmod 777 -R " + output_render);
	}
	//---------------------------------------------------

	QString cmd = "sh " + natron_module + " " + exe + " \"" + project[ins] + "\" " + renderNode[ins] + " \"" + output + "\" " + firstFrame + " " + lastFrame;

	mutex->unlock();

	QString log;
	log = qprocess(cmd, ins);
	QString log_file = path + "/log/render_log_" + QString::number(ins);
	fwrite(log_file, log);

	// post render
	if (log.contains("Rendering finished"))
		return true;
	else
		return false;
	//-----------------------------------------------
}

bool render_class::ae(int ins)
{
	QString log_file = path + "/log/render_log_" + QString::number(ins);
	os::remove(log_file);

	QString folderRender = os::dirname(extra[ins]);
	// Crea carpeta de renders
	if (not os::isdir(folderRender))
	{
		os::mkdir(folderRender);
		if (_linux)
			os::system("chmod 777 -R " + folderRender);
	}
	//---------------------------------------------------

	QString firstFrame = QString::number(first_frame[ins]);
	QString lastFrame = QString::number(last_frame[ins]);

	// crea numero con ceros para el nombre a partir del primer cuadro
	QString num = "0000000000" + firstFrame;
	QStringRef nameNumber(&num, num.length() - 10, 10);
	// -------------------------------------
	QString output = extra[ins] + "_" + nameNumber + ".mov";
	QString tmp = extra[ins] + "_" + nameNumber + "_tmp.mov";
	os::remove(output); // borra el mov antes del render

	QString args = "\"" + renderNode[ins] + "\" \"" + project[ins] + "\" \"" + output + "\" \"" + log_file + "\" " + firstFrame + " " + lastFrame + " " + QString::number(ins);

	args = args.replace(src_path[ins], dst_path[ins]);

	QString cmd = "sh " + path + "/modules/ae/aerender.sh " + args;
	// rendering ...
	// ----------------------------------
	qprocess(cmd, ins);
	// ----------------------------------

	// crea una version mas liviana ".mp4" del video y luego borra el original ".mov"
	QString postRender = "ffmpeg -i \"" + output + "\" -b:v 5M -c:a pcm_s16le " + tmp;
	os::sh(postRender);
	os::remove(output);
	os::rename(tmp, output);
	// ----------------------------------------

	// post render
	QString log = fread(log_file);
	if (log.contains("Finished composition"))
		return true;
	else
		return false;
	//-----------------------------------------------
}

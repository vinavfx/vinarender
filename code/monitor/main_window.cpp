#include "main_window.h"

void monitor::init(){

    //shared->server_display = {0,0,0,0,0,0,0};
    main_menu();
    tool_bar();
    log_ui();

	//ui();
	//notifyIcon();
    assamble();

    /*
	QSettings *settings = new QSettings("JumpCats", "CatsFarm");
	restoreGeometry( settings->value("geometry", "").toByteArray());
	restoreState( settings->value("windowState", "").toByteArray());

	bool status = 1;//tcpClient(host=managerHost, port=7772, timeout=5).status

	if ( status ){ startApp(); }
	else{
		bool ok;
		QString ip = QInputDialog::getText( this, "Manager Connect", "Enter Manager IP:", QLineEdit::Normal,
                                         QDir::home().dirName(), &ok);

		if ( ok ){
			fwrite( "../../etc/manager_host", ip.toStdString() );
			status = 1;//tcpClient(host=managerHost, port=7772, timeout=5).status

			if ( status ){ startApp(); }
			else{ closeApp(); }
		}
		else{ closeApp(); }
	}
    */

    qthread( &monitor::update, this );
}

void monitor::update(){
	// si el archivo esta en 1 muestra el monitor
	static string showMonitor = path() + "/etc/showMonitor";
	if ( not ( fread( showMonitor ) == "0" ) ){
		this->show();
		fwrite( showMonitor, "0" );
	} // ----------------------------
}

void monitor::assamble(){

	this->setCentralWidget( jobsList->widget );	
	this->addDockWidget( Qt::LeftDockWidgetArea, uiSubmit );
	this->addDockWidget( Qt::LeftDockWidgetArea, uiDenoiser );
	this->addDockWidget( Qt::BottomDockWidgetArea, uiServerOptions );
	this->addDockWidget( Qt::BottomDockWidgetArea, serverList->dock );
	this->addDockWidget( Qt::BottomDockWidgetArea, groupList->dock );
	this->addDockWidget( Qt::RightDockWidgetArea, taskList->dock );
	this->addToolBar( toolBar );
	this->addDockWidget( Qt::LeftDockWidgetArea, uiJobOptions );
	this->addDockWidget( Qt::LeftDockWidgetArea, log_dock );
}

void monitor::main_menu(){

	// Main Menu-------------------------------------
	QMenuBar *mainMenu = menuBar();
	QMenu *fileMenu = new QMenu("&File", this );
	mainMenu->addMenu( fileMenu );
	fileMenu->addAction( _general->hideAction );
	fileMenu->addAction( _general->quitAction );

	QMenu *editMenu = new QMenu("&Edit", this );
	mainMenu->addMenu( editMenu );
	editMenu->addAction( _general->preferencesAction );
	editMenu->addAction( _general->panelSubmitAction );
	editMenu->addAction( _general->hidePanelsAction );

	QMenu *serverMenu = new QMenu("&Servers", this );
	mainMenu->addMenu( serverMenu );
	serverMenu->addAction( serverActions->serverInactiveAction );
	serverMenu->addAction( serverActions->serverReactiveAction );
	serverMenu->addAction( serverActions->deleteAction );
	serverMenu->addSeparator();
	serverMenu->addAction( serverActions->serverMaxInstancesAction );
	serverMenu->addSeparator();
	serverMenu->addAction( serverActions->serverShowLog );
	serverMenu->addAction( serverActions->serverFreeramAction );
	serverMenu->addAction( serverActions->serverSshAction );

	QMenu *groupMenu = new QMenu("&Groups", this );
	mainMenu->addMenu( groupMenu );
	groupMenu->addAction( groupActions->groupCreateAction );
	groupMenu->addAction( groupActions->deleteAction );
	groupMenu->addSeparator();
	groupMenu->addAction( groupActions->groupAddmachineAction);

	QMenu *jobMenu = new QMenu("&Jobs", this );
	mainMenu->addMenu( jobMenu );	
	jobMenu->addAction( jobActions->jobResumeAction );
	jobMenu->addAction( jobActions->jobSuspendAction );
	jobMenu->addAction( jobActions->deleteAction );
	jobMenu->addAction( jobActions->jobRestartAction );
	jobMenu->addSeparator();
	jobMenu->addAction( jobActions->jobShowRenderFolderAction );
	jobMenu->addSeparator();
	jobMenu->addAction( jobActions->jobModifyAction );
	jobMenu->addAction( jobActions->jobLogAction );

	QMenu *taskMenu = new QMenu("&Tasks", this );
	mainMenu->addMenu( taskMenu );	
	taskMenu->addAction( taskActions->taskSuspendAction );
	taskMenu->addSeparator();
	taskMenu->addAction( taskActions->taskRestartAction );

	QMenu *helpMenu = new QMenu("&Help", this );
	mainMenu->addMenu( helpMenu );	
	helpMenu->addAction( _general->aboutAction );

	//--------------------------------------------------
}

void monitor::tool_bar(){

	// toolBar

	toolBar->setObjectName("ToolBar");
	toolBar->addAction( jobActions->jobResumeAction );
	toolBar->addSeparator();
	toolBar->addAction( jobActions->jobSuspendAction );
	toolBar->addSeparator();
	toolBar->addAction( _general->preferencesAction );
	toolBar->addSeparator();
	toolBar->addAction( _general->panelSubmitAction );
	toolBar->setIconSize(QSize(24,24));
	toolBar->setMovable(0);
	toolBar->setWindowTitle("Tools Bar");

	//---------------------
}

void monitor::log_ui(){

	kgl::QCodeEditorDesign design(":/design.xml");
	QList< kgl::QSyntaxRule > rules = kgl::QSyntaxRules::loadFromFile(":/rule_cpp.xml", design);

	log_text->setDesign(design);
	log_text->setRules(rules);
	log_text->setKeywords({ "printf", "scanf", "atoi", "mbtoa", "strlen", "memcpy", "memset" });
	log_text->setReadOnly( true );
	
	QWidget *widget = new QWidget();
	QVBoxLayout *hbox = new QVBoxLayout();
	hbox->addWidget( log_text );
	widget->setLayout( hbox );

	log_text->setObjectName("Logs");
	log_dock->setObjectName("Logs");
	log_dock->setWidget( widget );
	log_dock->hide();
}	

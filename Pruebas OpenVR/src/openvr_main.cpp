#include "scene_manager.h"

int main (int argc, char *argv[]) {

	SceneManager* sm = SceneManager::getInstance();
	int initOk = sm->init();

	if (initOk == 0)
	{
		while (!sm->getFin()) {
			sm->update();
			sm->draw();
		}
		sm->exit();
	}
	else
		cout << "Application initialization failed" << endl;

	return 0;
}


//     ____  __________     ___   ____ ___  _____ //
//    / __ \/ ____/ __ \   |__ \ / __ \__ \|__  / //
//   / /_/ / /   / / / /   __/ // / / /_/ / /_ <  //
//  / ____/ /___/ /_/ /   / __// /_/ / __/___/ /  //
// /_/    \____/\____/   /____/\____/____/____/   //
// Auteurs : Prénom Nom, Prénom Nom


// A vous de remplir les méthodes, vous pouvez ajouter des attributs ou méthodes pour vous aider
// déclarez les dans ComputationManager.h et définissez les méthodes ici.
// Certaines fonctions ci-dessous ont déjà un peu de code, il est à remplacer, il est là temporairement
// afin de faire attendre les threads appelants et aussi afin que le code compile.

#include "computationmanager.h"


ComputationManager::ComputationManager(int maxQueueSize): MAX_TOLERATED_QUEUE_SIZE(maxQueueSize)
{
    // TODO
	 id = 0;
	 bufferRequestsA = std::queue<Request>();
	 bufferRequestsB = std::queue<Request>();
	 bufferRequestsC = std::queue<Request>();

}

int ComputationManager::requestComputation(Computation c) {
	// TODO
	monitorIn();
	//il faut construire un request avec la computation brut afin de les trier
	//dans le buffer du résultat !
	switch (c.computationType){
		case ComputationType::A: {
			//on construit un request avec la computation brut
			Request r(c, id++);
			//on ajoute le request dans le buffer
			bufferRequestsA.push(r);
			//on notifie les threads qui attendent
			signal(condA);
			break;
		}
		case ComputationType::B:{
			//on construit un request avec la computation brut
			Request r(c, id++);
			//on ajoute le request dans le buffer
			bufferRequestsB.push(r);
			//on notifie les threads qui attendent
			signal(condB);
			break;
		}
		case ComputationType::C:{
			//on construit un request avec la computation brut
			Request r(c, id++);
			//on ajoute le request dans le buffer
			bufferRequestsC.push(r);
			//on notifie les threads qui attendent
			signal(condC);
			break;
		}
	}
	monitorOut();
}

void ComputationManager::abortComputation(int id) {
    // TODO
}

Result ComputationManager::getNextResult() {
    // TODO
    // Replace all of the code below by your code

    // Filled with some code in order to make the thread in the UI wait
    monitorIn();
    auto c = Condition();
    wait(c);
    monitorOut();

    return Result(-1, 0.0);
}

Request ComputationManager::getWork(ComputationType computationType) {
    // TODO
    // Replace all of the code below by your code

    // Filled with arbitrary code in order to make the callers wait
    monitorIn();
	switch (computationType) {
		case ComputationType::A: {
			//si le buffer est vide, on attend
			if (bufferRequestsA.empty()) {
				wait(condA);
			}
			//on récupère le request
			Request r = bufferRequestsA.front();
			//on le retire du buffer
			bufferRequestsA.pop();
			//on retourne le request
			monitorOut();
			return r;
		}
		case ComputationType::B: {
			//si le buffer est vide, on attend
			if (bufferRequestsB.empty()) {
				wait(condB);
			}
			//on récupère le request
			Request r = bufferRequestsB.front();
			//on le retire du buffer
			bufferRequestsB.pop();
			//on retourne le request
			monitorOut();
			return r;
		}
		case ComputationType::C: {
			//si le buffer est vide, on attend
			if (bufferRequestsC.empty()) {
				wait(condC);
			}
			//on récupère le request
			Request r = bufferRequestsC.front();
			//on le retire du buffer
			bufferRequestsC.pop();
			//on retourne le request
			monitorOut();
			return r;
		}
	}

}

bool ComputationManager::continueWork(int id) {
    // TODO
    return true;
}

void ComputationManager::provideResult(Result result) {
    // TODO
}

void ComputationManager::stop() {
    // TODO
}

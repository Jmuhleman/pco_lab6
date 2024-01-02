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
#include <algorithm>

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
			if (bufferRequestsA.size() >= MAX_TOLERATED_QUEUE_SIZE) {
				//si le buffer est plein, on attend
				wait(queueAFull);
			}
			//on ajoute le request dans le buffer
			bufferRequestsA.push(r);
			//on notifie les threads qui attendent
			signal(queueAEmpty);
			break;
		}
		case ComputationType::B:{
			//on construit un request avec la computation brut
			Request r(c, id++);
			if (bufferRequestsA.size() >= MAX_TOLERATED_QUEUE_SIZE) {
				//si le buffer est plein, on attend
				wait(queueBFull);
			}
			//on ajoute le request dans le buffer
			bufferRequestsB.push(r);
			//on notifie les threads qui attendent
			signal(queueBEmpty);
			break;
		}
		case ComputationType::C:{
			//on construit un request avec la computation brut
			Request r(c, id++);
			if (bufferRequestsA.size() >= MAX_TOLERATED_QUEUE_SIZE) {
				//si le buffer est plein, on attend
				wait(queueCFull);
			}
			//on ajoute le request dans le buffer
			bufferRequestsC.push(r);
			//on notifie les threads qui attendent
			signal(queueCEmpty);
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

	 if (bufferResults.empty()) {
		 wait(bufferEmpty);
	 }

     Result r = bufferResults.front();
	 bufferResults.erase(bufferResults.begin());
	 monitorOut();
	 return r;
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
				wait(queueAEmpty);
			}
			//on récupère le request
			Request r = bufferRequestsA.front();
			//on le retire du buffer
			bufferRequestsA.pop();
			//on retourne le request
			signal(queueAFull);
			monitorOut();
			return r;
		}
		case ComputationType::B: {
			//si le buffer est vide, on attend
			if (bufferRequestsB.empty()) {
				wait(queueBEmpty);
			}
			//on récupère le request
			Request r = bufferRequestsB.front();
			//on le retire du buffer
			bufferRequestsB.pop();
			//on retourne le request
			signal(queueBFull);
			monitorOut();
			return r;
		}
		case ComputationType::C: {
			//si le buffer est vide, on attend
			if (bufferRequestsC.empty()) {
				wait(queueCEmpty);
			}
			//on récupère le request
			Request r = bufferRequestsC.front();
			//on le retire du buffer
			bufferRequestsC.pop();
			//on retourne le request
			signal(queueBFull);
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

	monitorIn();
	bufferResults.push_back(result);
	std::sort(bufferResults.begin(), bufferResults.end(),
			  [](Result a, Result b) {return a.getId() < b.getId(); });
	signal(bufferEmpty);
	monitorOut();

}

void ComputationManager::stop() {
    // TODO
}

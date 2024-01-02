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
			bufferRequestsA.push_back(r);
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
			bufferRequestsB.push_back(r);
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
			bufferRequestsC.push_back(r);
			//on notifie les threads qui attendent
			signal(queueCEmpty);
			break;
		}
	}
	monitorOut();
}

void ComputationManager::abortComputation(int id) {
    // TODO
	//aller dans chaque queue et buffer de résultat et supprimer le request avec l'id
	//si le request est trouvé, on le supprime et on notifie les threads qui attendent
	//sinon, on ne fait rien
	//ajouter un vector des ids annulés pour vérifier si il faut continuer depuis
	// continueWork(id)
	monitorIn();
	// Fonction lambda pour supprimer un élément avec l'ID spécifié de la deque
	//TODO faire un truc générique ...
	auto removeIdDeque = [id](std::deque<Request>& requests) {

	   auto ret = requests.erase(std::remove_if(requests.begin(), requests.end(),
	                                            [id](const Request& request) { return request.getId() == id; }), requests.end());
	   return ret != requests.end();
	};

	auto removeIdVector = [id](std::vector<Result>& requests) {

	    auto ret = requests.erase(std::remove_if(requests.begin(), requests.end(),
	                                             [id](const Result& request) { return request.getId() == id; }), requests.end());
	   return ret != requests.end();
	};
	// Supprimer l'élément avec l'ID spécifié de chaque deque
	// Signaler que les files ne sont plus pleines
	if (removeIdDeque(bufferRequestsA)) {
		signal(queueAFull);
	}
	if (removeIdDeque(bufferRequestsB)) {
		signal(queueBFull);
	}
	if (removeIdDeque(bufferRequestsC)) {
		signal(queueCFull);
	}
	removeIdVector(bufferResults);
	abortedIds.push_back(id);
	monitorOut();

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
			bufferRequestsA.pop_front();
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
			bufferRequestsB.pop_front();
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
			bufferRequestsC.pop_front();
			//on retourne le request
			signal(queueBFull);
			monitorOut();
			return r;
		}
	}

}

bool ComputationManager::continueWork(int id) {
    // TODO
	if (std::any_of(abortedIds.begin(), abortedIds.end(), [id](int abortedId) {return abortedId == id; })) {
		abortedIds.erase(std::remove(abortedIds.begin(), abortedIds.end(), id), abortedIds.end());
		return false;
	}
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

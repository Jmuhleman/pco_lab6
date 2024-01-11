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

ComputationManager::ComputationManager(int maxQueueSize) : MAX_TOLERATED_QUEUE_SIZE(
	maxQueueSize) {
	idRequest = 0;
	requestStop = false;
	currentIdConsumption = 0;
	nWaitingOnQueueA = 0;
	nWaitingOnQueueB = 0;
	nWaitingOnQueueC = 0;
}

int ComputationManager::requestComputation(Computation c) {
	monitorIn();
	//il faut construire un request avec la computation brut afin de les trier
	//dans le buffer du résultat
	Request r(c, idRequest);
	switch (c.computationType) {
		case ComputationType::A: {
			//on construit un request avec la computation brut
			if (bufferRequestsA.size() >= MAX_TOLERATED_QUEUE_SIZE) {
				//si le buffer est plein, on attend
				if (requestStop) {
					monitorOut();
					throwStopException();
				}
				wait(queueAFull);
				if (requestStop) {
					monitorOut();
					throwStopException();
				}
			}
			//on ajoute le request dans le buffer
			bufferRequestsA.push_back(r);
			//on notifie les threads qui attendent
			signal(queueAEmpty);
			break;
		}
		case ComputationType::B: {
			//on construit un request avec la computation brut
			if (bufferRequestsB.size() >= MAX_TOLERATED_QUEUE_SIZE) {
				//si le buffer est plein, on attend
				if (requestStop) {
					monitorOut();
					throwStopException();
				}
				wait(queueBFull);
				if (requestStop) {
					monitorOut();
					throwStopException();
				}
			}
			//on ajoute le request dans le buffer
			bufferRequestsB.push_back(r);
			//on notifie les threads qui attendent
			signal(queueBEmpty);
			break;
		}
		case ComputationType::C: {
			//on construit un request avec la computation brut
			if (bufferRequestsC.size() >= MAX_TOLERATED_QUEUE_SIZE) {
				//si le buffer est plein, on attend
				if (requestStop) {
					monitorOut();
					throwStopException();
				}
				wait(queueCFull);
				if (requestStop) {
					monitorOut();
					throwStopException();
				}
			}
			//on ajoute le request dans le buffer
			bufferRequestsC.push_back(r);
			//on notifie les threads qui attendent
			signal(queueCEmpty);
		}
	}
	++idRequest;
	monitorOut();
	return r.getId();
}

/*
template <typename T>
void removeId(T &requests, const int id) {
	requests.erase(std::remove_if(
		requests.begin(),
		requests.end(),
		[id](const Request &request) {return request.getId() == id;},
		requests.end()));
}*/

void ComputationManager::abortComputation(int id) {
	//aller dans chaque queue et buffer de résultat et supprimer le request avec l'idRequest
	//si le request est trouvé, on le supprime et on notifie les threads qui attendent
	//sinon, on ne fait rien
	//ajouter un vector des ids annulés pour vérifier s'il faut continuer depuis
	// continueWork(idRequest)
	monitorIn();
	// A la demande du professeur j'ai laissé ces fonctions en version lambda
	// après avoir essayé de faire une version générique (ligne 98-104) sans
	// réussir...
	auto removeIdDeque = [id](std::deque<Request> &requests) {

	   requests.erase(std::remove_if(requests.begin(), requests.end(),
	                                 [id](const Request &request) {
		                                return request.getId() == id;
	                                 }), requests.end());
	};

	auto removeIdVector = [id](std::vector<Result> &requests) {

	   requests.erase(std::remove_if(requests.begin(), requests.end(),
	                                 [id](const Result &request) {
		                                return request.getId() == id;
	                                 }), requests.end());
	};
	// Supprimer l'élément avec l'ID spécifié de chaque deque
	// Signaler que les files ne sont plus pleines
	removeIdDeque(bufferRequestsA);
	removeIdDeque(bufferRequestsB);
	removeIdDeque(bufferRequestsC);

	removeIdVector(bufferResults);
	if (bufferRequestsA.size() < MAX_TOLERATED_QUEUE_SIZE) {
		signal(queueAFull);
	}
	if (bufferRequestsB.size() < MAX_TOLERATED_QUEUE_SIZE) {
		signal(queueBFull);
	}
	if (bufferRequestsC.size() < MAX_TOLERATED_QUEUE_SIZE) {
		signal(queueCFull);
	}
	abortedIds.push_back(id);
	monitorOut();
}

Result ComputationManager::getNextResult() {
	monitorIn();

	while (bufferResults.empty() || bufferResults.front().getId()
	                                != currentIdConsumption) {
		if (requestStop) {
			monitorOut();
			throwStopException();
		}
		wait(bufferNotReady);
		if (requestStop) {
			monitorOut();
			throwStopException();
		}
	}
	Result r = bufferResults.front();
	bufferResults.erase(bufferResults.begin());
	++currentIdConsumption;

	monitorOut();
	return r;
}

Request ComputationManager::getWork(ComputationType computationType) {
	monitorIn();
	Request r;
	switch (computationType) {
		case ComputationType::A: {
			//si le buffer est vide, on attend
			if (bufferRequestsA.empty()) {
				if (requestStop) {
					monitorOut();
					throwStopException();
				}
				++nWaitingOnQueueA;
				wait(queueAEmpty);
				if (requestStop) {
					monitorOut();
					throwStopException();
				}
				--nWaitingOnQueueA;
			}
			//on récupère le request
			r = bufferRequestsA.front();
			//on le retire du buffer
			bufferRequestsA.pop_front();
			//on retourne le request
			signal(queueAFull);
			break;
		}
		case ComputationType::B: {
			//si le buffer est vide, on attend
			if (bufferRequestsB.empty()) {
				if (requestStop) {
					monitorOut();
					throwStopException();
				}
				++nWaitingOnQueueB;
				wait(queueBEmpty);
				if (requestStop) {
					monitorOut();
					throwStopException();
				}
				--nWaitingOnQueueB;

			}
			//on récupère le request
			r = bufferRequestsB.front();
			//on le retire du buffer
			bufferRequestsB.pop_front();
			//on retourne le request
			signal(queueBFull);
			break;
		}
		case ComputationType::C: {
			//si le buffer est vide, on attend
			if (bufferRequestsC.empty()) {
				if (requestStop) {
					monitorOut();
					throwStopException();
				}
				++nWaitingOnQueueC;
				wait(queueCEmpty);
				if (requestStop) {
					monitorOut();
					throwStopException();
				}
				--nWaitingOnQueueC;

			}
			//on récupère le request
			r = bufferRequestsC.front();
			//on le retire du buffer
			bufferRequestsC.pop_front();
			//on retourne le request
			signal(queueCFull);
		}
	}
	monitorOut();
	return r;
}

bool ComputationManager::continueWork(int id) {
	if (requestStop) {
		return false;
	}
	monitorIn();
	if (std::any_of(abortedIds.begin(), abortedIds.end(),
	                [id](int abortedId) { return abortedId == id; })) {
		abortedIds.erase(std::remove(abortedIds.begin(), abortedIds.end(), id),
		                 abortedIds.end());
		monitorOut();
		return false;
	}
	monitorOut();
	return true;
}

void ComputationManager::provideResult(Result result) {
	monitorIn();
	bufferResults.push_back(result);

	std::sort(bufferResults.begin(), bufferResults.end(),
	          [](Result a, Result b) { return a.getId() < b.getId(); });

	if (bufferResults.front().getId() == currentIdConsumption) {
		signal(bufferNotReady);
	}
	monitorOut();
}

void ComputationManager::stop() {
	monitorIn();

	// selon les indications du professeur, on considère qu'il n'y a qu'un seul
	// client
	// faisant des demandes de calculs. Donc pas de compteurs sur les fonctions
	// interfaces du client faisant un wait puisque le client est unique.
	requestStop = true;
	for (int k = 0; k < nWaitingOnQueueA; ++k) {
		signal(queueAEmpty);
	}
	nWaitingOnQueueA = 0;
	for (int k = 0; k < nWaitingOnQueueB; ++k) {
		signal(queueBEmpty);
	}
	nWaitingOnQueueB = 0;
	for (int k = 0; k < nWaitingOnQueueC; ++k) {
		signal(queueCEmpty);
	}
	nWaitingOnQueueC = 0;

	signal(queueAFull);
	signal(queueBFull);
	signal(queueCFull);
	signal(bufferNotReady);
	monitorOut();
}

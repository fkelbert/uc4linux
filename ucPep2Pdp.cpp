#include "ucPep2Pdp.h"

#if UC_THRIFT

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace de::tum::in::i22::uc::thrift::types;

boost::shared_ptr<TTransport> tr;
TPep2PdpClient *cl;

void initPep2PdpThriftClient(int port) {
	boost::shared_ptr<TSocket> sock;

#if UC_THRIFT_SSL_ENABLED
	boost::shared_ptr<TSSLSocketFactory> factory = boost::shared_ptr<TSSLSocketFactory>(new TSSLSocketFactory());

	factory->loadTrustedCertificates(UC_THRIFT_SSL_CA);

	factory->authenticate(true);

	sock = factory->createSocket("localhost", port);

#else
	sock = boost::shared_ptr<TSocket>(new TSocket("localhost", port));
#endif

	boost::shared_ptr<TTransport> transport(new TBufferedTransport(sock));
	boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
	TPep2PdpClient *client = new TPep2PdpClient(protocol);

	tr = transport;
	cl = client;
}

bool connectPep2PdpThriftClient() {
	try {
		tr->open();
	} catch (TException& tx) {
		cout << "ERROR: " << tx.what() << endl;
		return false;
	}

	return true;
}

bool disconnectPep2PdpThriftClient() {
	try {
		tr->close();
	} catch (TException& tx) {
		cout << "ERROR: " << tx.what() << endl;
		return false;
	}


	return true;
}

void notifyEventToPdpThriftCpp(event *ev) {
	auto_ptr<TEvent> tev(new TEvent);
	tev->isActual = ev->isActual;
	tev->name = string(ev->name);

	for (int i = 0; i < ev->cntParams; i++) {
		tev->parameters.insert(make_pair(string(ev->params[i]->key), string(ev->params[i]->val)));
	}

	if (ev->isActual) {
#if UC_ONLY_EXECVE
		auto_ptr<TResponse> response(new TResponse);
		cl->notifyEventSync(*response, *tev);
#else
		cl->notifyEventAsync(*tev);
#endif
	}
	else {
		auto_ptr<TResponse> response(new TResponse);
		cl->notifyEventSync(*response, *tev);
	}
}

#endif

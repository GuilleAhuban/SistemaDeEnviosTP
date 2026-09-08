#include <iostream>
#include <string>
#include <utility>

namespace Envios {
    class Envio;
    class Movimiento;
    class NodoMovimiento;
    class Historial;
    class ListaPendientes;
    class NodoGlobal;
    class HubSystem;

    // Enum para los niveles de servicio
    enum NivelServicio {
        EXPRESS,
        PRIORITARIO,
        ESTANDAR
    };

    // Enum para los estados del envio
    enum EstadoEnvio {
        RECIBIDO,
        CLASIFICADO,
        EN_REPARTO,
        REPROGRAMADO,
        ENTREGADO
    };

    // Funciones auxiliares para convertir enums a string para la consola
    std::string servicioToString(NivelServicio s) {
        switch (s) {
            case EXPRESS: return "EXPRESS";
            case PRIORITARIO: return "PRIORITARIO";
            case ESTANDAR: return "ESTANDAR";
            default: return "DESCONOCIDO";
        }
    }

    std::string estadoToString(EstadoEnvio e) {
        switch (e) {
            case RECIBIDO: return "RECIBIDO";
            case CLASIFICADO: return "CLASIFICADO";
            case EN_REPARTO: return "EN_REPARTO";
            case REPROGRAMADO: return "REPROGRAMADO";
            case ENTREGADO: return "ENTREGADO";
            default: return "DESCONOCIDO";
        }
    }

    class Movimiento {
    private:
        int numeroSecuencial;
        EstadoEnvio estado;
        std::string observacion;

    public:
        Movimiento(int num, EstadoEnvio est, std::string obs)
            : numeroSecuencial(num), estado(est), observacion(std::move(obs)) {}

        int getNumeroSecuencial() const { return numeroSecuencial; }
        EstadoEnvio getEstado() const { return estado; }
        std::string getObservacion() const { return observacion; }

        void mostrar() const {
            std::cout << numeroSecuencial << " | "
                      << estadoToString(estado) << " | "
                      << observacion << "\n";
        }
    };

    class NodoMovimiento {
    public:
        Movimiento dato;
        NodoMovimiento* siguiente;
        NodoMovimiento* anterior;

        NodoMovimiento(Movimiento m)
            : dato(std::move(m)), siguiente(nullptr), anterior(nullptr) {}
    };

    class Historial {
    private:
        NodoMovimiento* cabeza;
        NodoMovimiento* cola;
        int contadorMovimientos;

    public:
        Historial() : cabeza(nullptr), cola(nullptr), contadorMovimientos(0) {}

        NodoMovimiento* getCabeza() const { return cabeza; }
        NodoMovimiento* getCola() const { return cola; }
        int getCantidadMovimientos() const { return contadorMovimientos; }

        ~Historial() {
            NodoMovimiento* actual = cabeza;
            while (actual != nullptr) {
                NodoMovimiento* aBorrar = actual;
                actual = actual->siguiente;
                delete aBorrar;
            }
        }

        void agregarMovimiento(EstadoEnvio estado, const std::string& observacion) {
            contadorMovimientos++;
            Movimiento nuevoMov(contadorMovimientos, estado, observacion);
            auto* nuevoNodo = new NodoMovimiento(nuevoMov);

            if (cabeza == nullptr) {
                cabeza = nuevoNodo;
                cola = nuevoNodo;
            } else {
                cola->siguiente = nuevoNodo;
                nuevoNodo->anterior = cola;
                cola = nuevoNodo;
            }
        }

        void mostrarCronologico() const {
            NodoMovimiento* actual = cabeza;
            if (!actual) {
                std::cout << "  (Historial vacio)\n";
                return;
            }
            while (actual != nullptr) {
                std::cout << "  ";
                actual->dato.mostrar();
                actual = actual->siguiente;
            }
        }

        void mostrarInverso() const {
            NodoMovimiento* actual = cola;
            if (!actual) {
                std::cout << "  (Historial vacio)\n";
                return;
            }
            while (actual != nullptr) {
                std::cout << "  ";
                actual->dato.mostrar();
                actual = actual->anterior;
            }
        }
    };

    class Envio {
    private:
        std::string codigo;
        std::string destinatario;
        std::string zona;
        double peso;
        NivelServicio servicio;
        EstadoEnvio estadoActual;
        int intentosEntrega;
        Historial historial;

    public:
        Envio(std::string cod, std::string dest, std::string z,
              double p, const NivelServicio serv)
            : codigo(std::move(cod)), destinatario(std::move(dest)), zona(std::move(z)), peso(p),
              servicio(serv), estadoActual(RECIBIDO), intentosEntrega(0) {
            // Evento inicial obligatorio: RECIBIDO
            historial.agregarMovimiento(RECIBIDO, "Ingreso al centro de distribucion");
        }

        std::string getCodigo() const { return codigo; }
        std::string getZona() const { return zona; }
        double getPeso() const { return peso; }
        NivelServicio getServicio() const { return servicio; }
        EstadoEnvio getEstadoActual() const { return estadoActual; }
        int getIntentosEntrega() const { return intentosEntrega; }
        const Historial& getHistorial() const { return historial; }

        void cambiarEstado(EstadoEnvio nuevoEstado, const std::string& observacion) {
            estadoActual = nuevoEstado;
            historial.agregarMovimiento(nuevoEstado, observacion);
        }

        void registrarIntentoFallido(const std::string& motivo) {
            intentosEntrega++;
            estadoActual = REPROGRAMADO;
            historial.agregarMovimiento(REPROGRAMADO, motivo);
        }

        void mostrarHistorial(bool ordenInverso = false) const {
            std::cout << "--- Historial del envio " << codigo << " ---\n";
            if (ordenInverso) {
                historial.mostrarInverso();
            } else {
                historial.mostrarCronologico();
            }
        }

        void mostrarResumen() const {
            std::cout << codigo << " | "
                      << zona << " | "
                      << peso << " kg | "
                      << servicioToString(servicio) << " | "
                      << estadoToString(estadoActual) << " | Intentos: "
                      << intentosEntrega << "\n";
        }
    };

    class NodoPendiente {
    public:
        Envio* paquete;
        NodoPendiente* siguiente;

        NodoPendiente(Envio* env) : paquete(env), siguiente(nullptr) {}
    };

    class NodoGlobal {
    public:
        Envio* paquete;
        NodoGlobal* siguiente;

        NodoGlobal(Envio* env) : paquete(env), siguiente(nullptr) {}
    };

    struct ResumenZona {
        int cantidadPaquetes;
        double pesoTotal;
        int cantidadExpress;

        ResumenZona() : cantidadPaquetes(0), pesoTotal(0.0), cantidadExpress(0) {}
    };

    class ListaPendientes {
    private:
        NodoPendiente* cabeza;
        int cantidadNodos;

        static ResumenZona calcularResumenRecursivo(const NodoPendiente* nodoActual, const std::string& zonaBuscada) {
            if (nodoActual == nullptr) {
                return {};
            }

            ResumenZona resultadoSublista = calcularResumenRecursivo(nodoActual->siguiente, zonaBuscada);

            if (nodoActual->paquete->getZona() == zonaBuscada) {
                resultadoSublista.cantidadPaquetes += 1;
                resultadoSublista.pesoTotal += nodoActual->paquete->getPeso();
                if (nodoActual->paquete->getServicio() == EXPRESS) {
                    resultadoSublista.cantidadExpress += 1;
                }
            }

            return resultadoSublista;
        }

    public:
        ListaPendientes() : cabeza(nullptr), cantidadNodos(0) {}

        ~ListaPendientes() {
            NodoPendiente* actual = cabeza;
            while (actual != nullptr) {
                NodoPendiente* aBorrar = actual;
                actual = actual->siguiente;
                delete aBorrar;
            }
        }

        NodoPendiente* getCabeza() const { return cabeza; }
        bool estaVacia() const { return cabeza == nullptr; }
        int cantidad() const { return cantidadNodos; }

        void insertarOrdenado(Envio* nuevoEnvio) {
            auto* nuevoNodo = new NodoPendiente(nuevoEnvio);
            cantidadNodos++;

            if (cabeza == nullptr || nuevoEnvio->getServicio() < cabeza->paquete->getServicio()) {
                nuevoNodo->siguiente = cabeza;
                cabeza = nuevoNodo;
                return;
            }

            NodoPendiente* actual = cabeza;
            while (actual->siguiente != nullptr &&
                   actual->siguiente->paquete->getServicio() <= nuevoEnvio->getServicio()) {
                actual = actual->siguiente;
            }

            nuevoNodo->siguiente = actual->siguiente;
            actual->siguiente = nuevoNodo;
        }

        Envio* despacharProximo() {
            if (cabeza == nullptr) return nullptr;

            NodoPendiente* nodoAExtraer = cabeza;
            Envio* paqueteDespachado = nodoAExtraer->paquete;

            cabeza = cabeza->siguiente;
            delete nodoAExtraer;
            cantidadNodos--;

            return paqueteDespachado;
        }

        void mostrarPendientes() const {
            if (cabeza == nullptr) {
                std::cout << "No hay envios pendientes.\n";
                return;
            }

            NodoPendiente* actual = cabeza;
            while (actual != nullptr) {
                actual->paquete->mostrarResumen();
                actual = actual->siguiente;
            }
        }

        ResumenZona obtenerResumenZonaRecursivo(const std::string& zonaBuscada) const {
            return calcularResumenRecursivo(cabeza, zonaBuscada);
        }
    };

    class HubSystem {
    private:
        NodoGlobal* cabezaGlobal;
        ListaPendientes pendientes;

    public:
        HubSystem() : cabezaGlobal(nullptr) {}

        ~HubSystem() {
            NodoGlobal* actual = cabezaGlobal;
            while (actual != nullptr) {
                NodoGlobal* aBorrar = actual;
                actual = actual->siguiente;
                delete aBorrar->paquete; // Destruimos el envío almacenado
                delete aBorrar;          // Destruimos el nodo de la lista global
            }
        }

        void registrarEnvio(const std::string& cod, const std::string& dest,
                            const std::string& zona, double peso, NivelServicio serv) {

            //  nace como RECIBIDO y guarda evento inicial
            auto* nuevo = new Envio(cod, dest, zona, peso, serv);

            auto* nuevoNodoGlobal = new NodoGlobal(nuevo);
            nuevoNodoGlobal->siguiente = cabezaGlobal;
            cabezaGlobal = nuevoNodoGlobal;


            nuevo->cambiarEstado(CLASIFICADO, "Clasificado y asignado a bahia de despacho por prioridad");

              pendientes.insertarOrdenado(nuevo);
        }

        Envio* buscarEnvio(const std::string& cod) const {
            NodoGlobal* actual = cabezaGlobal;
            while (actual != nullptr) {
                if (actual->paquete->getCodigo() == cod) {
                    return actual->paquete;
                }
                actual = actual->siguiente;
            }
            return nullptr;
        }

        void mostrarPendientes() const {
            pendientes.mostrarPendientes();
        }

        void despacharProximo() {
            Envio* paquete = pendientes.despacharProximo();
            if (paquete == nullptr) {
                std::cout << "\n[!] No hay envios pendientes para despachar.\n";
                return;
            }

            // Pasa de CLASIFICADO a EN_REPARTO
            paquete->cambiarEstado(EN_REPARTO, "Asignado a unidad de reparto");
            std::cout << "\n[✓] Envio " << paquete->getCodigo() << " despachado con exito.\n";
        }

        void reprogramarEnvio(const std::string& codigo, const std::string& motivo) {
            Envio* paquete = buscarEnvio(codigo);
            if (paquete == nullptr) {
                std::cout << "\n[!] Envio no encontrado.\n";
                return;
            }

            // Impide reprogramar paquetes ya enviados
            if (paquete->getEstadoActual() == ENTREGADO) {
                std::cout << "\n[!] Error: No se puede reprogramar un envio que ya fue ENTREGADO.\n";
                return;
            }

            if (paquete->getEstadoActual() != EN_REPARTO) {
                std::cout << "\n[!] Solo se pueden reprogramar envios que esten EN_REPARTO.\n";
                return;
            }

            // marcar como REPROGRAMADO con el motivo de la falla
            paquete->registrarIntentoFallido(motivo);

            paquete->cambiarEstado(CLASIFICADO, "Re-clasificado para nuevo intento de entrega");

            // reinsertar en la lista de pendientes respetando su prioridad
            pendientes.insertarOrdenado(paquete);
            std::cout << "\n[✓] Envio " << codigo << " reprogramado, re-clasificado y reinsertado en pendientes.\n";
        }

        void finalizarEntrega(const std::string& codigo) const {
            Envio* paquete = buscarEnvio(codigo);
            if (paquete == nullptr) {
                std::cout << "\n[!] Envio no encontrado.\n";
                return;
            }

            if (paquete->getEstadoActual() != EN_REPARTO) {
                std::cout << "\n[!] Solo se pueden entregar envios que esten actualmente EN_REPARTO.\n";
                return;
            }

            paquete->cambiarEstado(ENTREGADO, "Entrega confirmada con el destinatario");
            std::cout << "\n[✓] Envio " << codigo << " marcado como ENTREGADO definitivamente.\n";
        }

        void consultarResumenZona(const std::string& zona) const {
            ResumenZona r = pendientes.obtenerResumenZonaRecursivo(zona);
            std::cout << "\n=== Resumen de Pendientes para Zona: " << zona << " ===\n";
            std::cout << "Cantidad de paquetes: " << r.cantidadPaquetes << "\n";
            std::cout << "Peso total pendiente: " << r.pesoTotal << " kg\n";
            std::cout << "Cantidad EXPRESS: " << r.cantidadExpress << "\n";
        }
    };

    void cargarDatasetInicial(HubSystem& sistema) {
        sistema.registrarEnvio("PKG-1001", "Ana Torres",    "CENTRO", 1.20, ESTANDAR);
        sistema.registrarEnvio("PKG-1002", "Bruno Díaz",    "NORTE",  0.75, EXPRESS);
        sistema.registrarEnvio("PKG-1003", "Carla Ruiz",    "SUR",    4.10, PRIORITARIO);
        sistema.registrarEnvio("PKG-1004", "Diego López",   "CENTRO", 2.30, ESTANDAR);
        sistema.registrarEnvio("PKG-1005", "Elena Castro",  "NORTE",  1.90, PRIORITARIO);
        sistema.registrarEnvio("PKG-1006", "Franco Gómez",  "SUR",    0.50, EXPRESS);
        sistema.registrarEnvio("PKG-1007", "Gabriela Soto", "CENTRO", 6.20, ESTANDAR);
        sistema.registrarEnvio("PKG-1008", "Hugo Pérez",    "NORTE",  3.40, PRIORITARIO);
    }
}

using namespace Envios;
#include <sstream>
#include <cstdlib>
#undef NDEBUG
#include <cassert>
#include <iostream>

namespace EnviosTests {

    int testsEjecutados = 0;

    void correr(const char* nombre, void (*test)()) {
        test();
        ++testsEjecutados;
        std::cerr << "[OK] " << nombre << "\n";
    }
    #define RUN_TEST(fn) correr(#fn, fn)

    class CapturaSalida {
    private:
        std::ostringstream buffer;
        std::streambuf* original;
    public:
        CapturaSalida() : original(std::cout.rdbuf(buffer.rdbuf())) {}
        ~CapturaSalida() { std::cout.rdbuf(original); }
        std::string texto() const { return buffer.str(); }
    };

    bool contiene(const std::string& texto, const std::string& fragmento) {
        return texto.find(fragmento) != std::string::npos;
    }

    bool apareceAntes(const std::string& texto, const std::string& a, const std::string& b) {
        std::string::size_type pa = texto.find(a);
        std::string::size_type pb = texto.find(b);
        return pa != std::string::npos && pb != std::string::npos && pa < pb;
    }

    std::string codigosPendientes(const ListaPendientes& lista) {
        std::string resultado;
        for (const NodoPendiente* n = lista.getCabeza(); n != nullptr; n = n->siguiente) {
            if (!resultado.empty()) resultado += ",";
            resultado += n->paquete->getCodigo();
        }
        return resultado;
    }

    void test_servicioToString() {
        assert(servicioToString(EXPRESS) == "EXPRESS");
        assert(servicioToString(PRIORITARIO) == "PRIORITARIO");
        assert(servicioToString(ESTANDAR) == "ESTANDAR");
        assert(servicioToString(static_cast<NivelServicio>(99)) == "DESCONOCIDO");
    }

    void test_estadoToString() {
        assert(estadoToString(RECIBIDO) == "RECIBIDO");
        assert(estadoToString(CLASIFICADO) == "CLASIFICADO");
        assert(estadoToString(EN_REPARTO) == "EN_REPARTO");
        assert(estadoToString(REPROGRAMADO) == "REPROGRAMADO");
        assert(estadoToString(ENTREGADO) == "ENTREGADO");
        assert(estadoToString(static_cast<EstadoEnvio>(99)) == "DESCONOCIDO");
    }

    void test_ordenPrioridadDelEnum() {
        assert(EXPRESS < PRIORITARIO);
        assert(PRIORITARIO < ESTANDAR);
    }

    void test_movimiento_getters() {
        Movimiento m(3, EN_REPARTO, "Asignado al movil 12");
        assert(m.getNumeroSecuencial() == 3);
        assert(m.getEstado() == EN_REPARTO);
        assert(m.getObservacion() == "Asignado al movil 12");
    }

    void test_movimiento_mostrar() {
        Movimiento m(4, REPROGRAMADO, "Destinatario ausente");
        CapturaSalida cap;
        m.mostrar();
        assert(cap.texto() == "4 | REPROGRAMADO | Destinatario ausente\n");
    }

    void test_nodoMovimiento_inicializaEnlacesEnNull() {
        NodoMovimiento nodo(Movimiento(1, RECIBIDO, "x"));
        assert(nodo.siguiente == nullptr);
        assert(nodo.anterior == nullptr);
        assert(nodo.dato.getNumeroSecuencial() == 1);
    }

    void test_nodoMovimiento_enlaceDobleManual() {
        auto* a = new NodoMovimiento(Movimiento(1, RECIBIDO, "a"));
        auto* b = new NodoMovimiento(Movimiento(2, CLASIFICADO, "b"));
        auto* c = new NodoMovimiento(Movimiento(3, EN_REPARTO, "c"));
        a->siguiente = b; b->anterior = a;
        b->siguiente = c; c->anterior = b;

        assert(a->siguiente->siguiente->dato.getNumeroSecuencial() == 3);
        assert(c->anterior->anterior->dato.getNumeroSecuencial() == 1);
        assert(c->siguiente == nullptr);
        assert(a->anterior == nullptr);

        delete a; delete b; delete c;
    }

    void test_historial_vacio() {
        Historial h;
        assert(h.getCabeza() == nullptr);
        assert(h.getCola() == nullptr);
        assert(h.getCantidadMovimientos() == 0);
        {
            CapturaSalida cap;
            h.mostrarCronologico();
            assert(cap.texto() == "  (Historial vacio)\n");
        }
        {
            CapturaSalida cap;
            h.mostrarInverso();
            assert(cap.texto() == "  (Historial vacio)\n");
        }
    }

    void test_historial_unMovimiento() {
        Historial h;
        h.agregarMovimiento(RECIBIDO, "Ingreso");
        assert(h.getCabeza() != nullptr);
        assert(h.getCabeza() == h.getCola());
        assert(h.getCabeza()->anterior == nullptr);
        assert(h.getCabeza()->siguiente == nullptr);
        assert(h.getCantidadMovimientos() == 1);
        CapturaSalida cap1;
        h.mostrarCronologico();
        std::string cron = cap1.texto();
        CapturaSalida cap2;
        h.mostrarInverso();
        std::string inv = cap2.texto();
        assert(cron == "  1 | RECIBIDO | Ingreso\n");
        assert(inv == cron);
    }

    void test_historial_numeracionSecuencial() {
        Historial h;
        h.agregarMovimiento(RECIBIDO, "uno");
        h.agregarMovimiento(CLASIFICADO, "dos");
        h.agregarMovimiento(EN_REPARTO, "tres");
        CapturaSalida cap;
        h.mostrarCronologico();
        assert(cap.texto() == "  1 | RECIBIDO | uno\n"
                            "  2 | CLASIFICADO | dos\n"
                            "  3 | EN_REPARTO | tres\n");
    }

    void test_historial_ejemploEnunciado_cronologicoEInverso() {
        Historial h;
        h.agregarMovimiento(RECIBIDO,     "Ingreso al centro de distribucion");
        h.agregarMovimiento(CLASIFICADO,  "Clasificado para zona NORTE");
        h.agregarMovimiento(EN_REPARTO,   "Asignado al movil 12");
        h.agregarMovimiento(REPROGRAMADO, "Destinatario ausente");
        h.agregarMovimiento(EN_REPARTO,   "Segundo intento");
        h.agregarMovimiento(ENTREGADO,    "Entrega confirmada");

        {
            CapturaSalida cap;
            h.mostrarCronologico();
            assert(cap.texto() == "  1 | RECIBIDO | Ingreso al centro de distribucion\n"
                                 "  2 | CLASIFICADO | Clasificado para zona NORTE\n"
                                 "  3 | EN_REPARTO | Asignado al movil 12\n"
                                 "  4 | REPROGRAMADO | Destinatario ausente\n"
                                 "  5 | EN_REPARTO | Segundo intento\n"
                                 "  6 | ENTREGADO | Entrega confirmada\n");
        }
        {
            CapturaSalida cap;
            h.mostrarInverso();
            assert(cap.texto() == "  6 | ENTREGADO | Entrega confirmada\n"
                                 "  5 | EN_REPARTO | Segundo intento\n"
                                 "  4 | REPROGRAMADO | Destinatario ausente\n"
                                 "  3 | EN_REPARTO | Asignado al movil 12\n"
                                 "  2 | CLASIFICADO | Clasificado para zona NORTE\n"
                                 "  1 | RECIBIDO | Ingreso al centro de distribucion\n");
        }
    }

    void test_historial_enlacesDobles() {
        Historial h;
        h.agregarMovimiento(RECIBIDO,    "uno");
        h.agregarMovimiento(CLASIFICADO, "dos");
        h.agregarMovimiento(EN_REPARTO,  "tres");
        h.agregarMovimiento(ENTREGADO,   "cuatro");
        assert(h.getCantidadMovimientos() == 4);

        assert(h.getCabeza()->anterior == nullptr);
        assert(h.getCola()->siguiente == nullptr);
        assert(h.getCabeza()->dato.getNumeroSecuencial() == 1);
        assert(h.getCola()->dato.getNumeroSecuencial() == 4);

        int esperado = 1;
        const NodoMovimiento* previo = nullptr;
        for (const NodoMovimiento* n = h.getCabeza(); n != nullptr; n = n->siguiente) {
            assert(n->dato.getNumeroSecuencial() == esperado);
            assert(n->anterior == previo);
            previo = n;
            ++esperado;
        }
        assert(esperado == 5);
        assert(previo == h.getCola());

        esperado = 4;
        const NodoMovimiento* posterior = nullptr;
        for (const NodoMovimiento* n = h.getCola(); n != nullptr; n = n->anterior) {
            assert(n->dato.getNumeroSecuencial() == esperado);
            assert(n->siguiente == posterior);
            posterior = n;
            --esperado;
        }
        assert(esperado == 0);
        assert(posterior == h.getCabeza());
    }

    void test_historial_agregarDespuesDeRecorrer() {
        Historial h;
        h.agregarMovimiento(RECIBIDO, "a");
        { CapturaSalida cap; h.mostrarInverso(); }
        h.agregarMovimiento(CLASIFICADO, "b");
        CapturaSalida cap;
        h.mostrarInverso();
        assert(cap.texto() == "  2 | CLASIFICADO | b\n  1 | RECIBIDO | a\n");
    }

    void test_historial_muchosMovimientos_destructor() {
        auto* h = new Historial();
        for (int i = 0; i < 5000; ++i) h->agregarMovimiento(EN_REPARTO, "intento");
        assert(h->getCantidadMovimientos() == 5000);
        assert(h->getCola()->dato.getNumeroSecuencial() == 5000);
        int contados = 0;
        for (const NodoMovimiento* n = h->getCola(); n != nullptr; n = n->anterior) ++contados;
        assert(contados == 5000);
        delete h;
    }
}

using namespace::Envios;

int main() {
    HubSystem sistema;
    cargarDatasetInicial(sistema);

    int opcion = 0;
    do {
        std::cout << "\n========== HUBFLOW Logistica ==========\n";
        std::cout << "1. Mostrar envios pendientes\n";
        std::cout << "2. Registrar nuevo envio\n";
        std::cout << "3. Buscar envio por codigo\n";
        std::cout << "4. Despachar proximo envio\n";
        std::cout << "5. Reprogramar envio fallido\n";
        std::cout << "6. Finalizar entrega\n";
        std::cout << "7. Mostrar historial (cronologico e Inverso)\n";
        std::cout << "8. Obtener resumen recursivo por zona\n";
        std::cout << "9. Salir\n";
        std::cout << "Seleccione una Opcion: ";
        std::cin >> opcion;

        std::string cod, zona, dest, motivo;
        int servOpt;
        double peso;

        switch (opcion) {
            case 1:
                std::cout << "\n--- ENVIOS PENDIENTES (Ordenados por Prioridad) ---\n";
                sistema.mostrarPendientes();
                break;

            case 2:
                std::cout << "\ncodigo: "; std::cin >> cod;
                if (sistema.buscarEnvio(cod) != nullptr) {
                    std::cout << "[!] Error: Ya existe un envio con ese codigo.\n";
                    break;
                }
                std::cout << "Destinatario: "; std::cin.ignore(); std::getline(std::cin, dest);
                std::cout << "Zona (NORTE/CENTRO/SUR): "; std::cin >> zona;
                std::cout << "Peso (kg): "; std::cin >> peso;
                std::cout << "Servicio (0: EXPRESS, 1: PRIORITARIO, 2: ESTANDAR): "; std::cin >> servOpt;

                sistema.registrarEnvio(cod, dest, zona, peso, static_cast<NivelServicio>(servOpt));
                std::cout << "[✓] Envio registrado e ingresado a pendientes.\n";
                break;

            case 3:
                std::cout << "\nIngrese el codigo del envio: "; std::cin >> cod;
            {
                Envio* e = sistema.buscarEnvio(cod);
                if (e) {
                    std::cout << "\n--- Datos del envio ---\n";
                    e->mostrarResumen();
                } else {
                    std::cout << "[!] Envio no encontrado.\n";
                }
            }
                break;

            case 4:
                sistema.despacharProximo();
                break;

            case 5:
                std::cout << "\ncodigo del envio a reprogramar: "; std::cin >> cod;
                std::cout << "Motivo del fallo: "; std::cin.ignore(); std::getline(std::cin, motivo);
                sistema.reprogramarEnvio(cod, motivo);
                break;

            case 6:
                std::cout << "\ncodigo del envio a entregar: "; std::cin >> cod;
                sistema.finalizarEntrega(cod);
                break;

            case 7:
                std::cout << "\ncodigo del envio: "; std::cin >> cod;
            {
                Envio* e = sistema.buscarEnvio(cod);
                if (e) {
                    int ordenOpt;
                    std::cout << "Orden (0: cronologico, 1: Inverso): "; std::cin >> ordenOpt;
                    e->mostrarHistorial(ordenOpt == 1);
                } else {
                    std::cout << "[!] Envio no encontrado.\n";
                }
            }
                break;

            case 8:
                std::cout << "\nIngrese la zona a consultar (ej. NORTE): "; std::cin >> zona;
                sistema.consultarResumenZona(zona);
                break;

            case 9:
                std::cout << "\nCerrando el sistema HubFlow y liberando memoria...\n";
                break;

            default:
                std::cout << "\nOpcion invalida. Intente de nuevo.\n";
        }
    } while (opcion != 9);

    std::cout << "Iniciando tests...\n";
    using namespace EnviosTests;

    RUN_TEST(test_servicioToString);
    RUN_TEST(test_estadoToString);
    RUN_TEST(test_ordenPrioridadDelEnum);
    RUN_TEST(test_movimiento_getters);
    RUN_TEST(test_movimiento_mostrar);
    RUN_TEST(test_nodoMovimiento_inicializaEnlacesEnNull);
    RUN_TEST(test_nodoMovimiento_enlaceDobleManual);
    RUN_TEST(test_historial_vacio);
    RUN_TEST(test_historial_unMovimiento);
    RUN_TEST(test_historial_numeracionSecuencial);
    RUN_TEST(test_historial_ejemploEnunciado_cronologicoEInverso);
    RUN_TEST(test_historial_enlacesDobles);
    RUN_TEST(test_historial_agregarDespuesDeRecorrer);
    RUN_TEST(test_historial_muchosMovimientos_destructor);

    std::cout << "\n>>> Se ejecutaron " << testsEjecutados << " tests correctamente. <<<\n";
    return 0;

}
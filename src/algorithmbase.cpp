#include "algorithmbase.h"

AlgorithmBase::AlgorithmBase(Graph *graph, QObject *parent)
    : QObject(parent), m_graph(graph), m_startVertex(-1), m_finished(false)
{
}

AlgorithmBase::~AlgorithmBase() = default;

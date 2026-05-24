// Serial-only compatibility stubs for assembly paths removed from this extract.

#include "bilininteg.hpp"
#include "lininteg.hpp"
#include "nonlininteg.hpp"

namespace mfem
{

namespace
{
void SerialAssemblyOnly()
{
   MFEM_ABORT("serial mini-MFEM keeps only legacy sparse assembly");
}
}

DomainLFIntegrator::Kernels::Kernels() { }

DiffusionIntegrator::Kernels::Kernels() { }
void DiffusionIntegrator::AssemblePA(const FiniteElementSpace&) { SerialAssemblyOnly(); }
void DiffusionIntegrator::AssembleNURBSPA(const FiniteElementSpace&) { SerialAssemblyOnly(); }
void DiffusionIntegrator::AssembleEA(const FiniteElementSpace&, Vector&, const bool) { SerialAssemblyOnly(); }
void DiffusionIntegrator::AssembleMF(const FiniteElementSpace&) { SerialAssemblyOnly(); }
void DiffusionIntegrator::AssembleDiagonalPA(Vector&) { SerialAssemblyOnly(); }
void DiffusionIntegrator::AssembleDiagonalMF(Vector&) { SerialAssemblyOnly(); }
void DiffusionIntegrator::AssemblePatchMatrix(const int, const FiniteElementSpace&, SparseMatrix*&) { SerialAssemblyOnly(); }
void DiffusionIntegrator::AddMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void DiffusionIntegrator::AddAbsMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void DiffusionIntegrator::AddMultTransposePA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void DiffusionIntegrator::AddAbsMultTransposePA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void DiffusionIntegrator::AddMultMF(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void DiffusionIntegrator::AddMultNURBSPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }

MassIntegrator::Kernels::Kernels() { }
void MassIntegrator::AssemblePA(const FiniteElementSpace&) { SerialAssemblyOnly(); }
void MassIntegrator::AssemblePABoundary(const FiniteElementSpace&) { SerialAssemblyOnly(); }
void MassIntegrator::AssembleEA(const FiniteElementSpace&, Vector&, const bool) { SerialAssemblyOnly(); }
void MassIntegrator::AssembleEABoundary(const FiniteElementSpace&, Vector&, const bool) { SerialAssemblyOnly(); }
void MassIntegrator::AssembleMF(const FiniteElementSpace&) { SerialAssemblyOnly(); }
void MassIntegrator::AssembleDiagonalPA(Vector&) { SerialAssemblyOnly(); }
void MassIntegrator::AssembleDiagonalMF(Vector&) { SerialAssemblyOnly(); }
void MassIntegrator::AddMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void MassIntegrator::AddAbsMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void MassIntegrator::AddMultTransposePA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void MassIntegrator::AddAbsMultTransposePA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void MassIntegrator::AddMultMF(const Vector&, Vector&) const { SerialAssemblyOnly(); }

ConvectionIntegrator::Kernels::Kernels() { }
void ConvectionIntegrator::AssemblePA(const FiniteElementSpace&) { SerialAssemblyOnly(); }
void ConvectionIntegrator::AssembleEA(const FiniteElementSpace&, Vector&, const bool) { SerialAssemblyOnly(); }
void ConvectionIntegrator::AssembleMF(const FiniteElementSpace&) { SerialAssemblyOnly(); }
void ConvectionIntegrator::AssembleDiagonalPA(Vector&) { SerialAssemblyOnly(); }
void ConvectionIntegrator::AssembleDiagonalMF(Vector&) { SerialAssemblyOnly(); }
void ConvectionIntegrator::AddMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void ConvectionIntegrator::AddMultTransposePA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void ConvectionIntegrator::AddMultMF(const Vector&, Vector&) const { SerialAssemblyOnly(); }

CurlCurlIntegrator::Kernels::Kernels() { }
void CurlCurlIntegrator::AssemblePA(const FiniteElementSpace&) { SerialAssemblyOnly(); }
void CurlCurlIntegrator::AssembleDiagonalPA(Vector&) { SerialAssemblyOnly(); }
void CurlCurlIntegrator::AddMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void CurlCurlIntegrator::AddAbsMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }

void ElasticityIntegrator::AssemblePA(const FiniteElementSpace&) { SerialAssemblyOnly(); }
void ElasticityIntegrator::AssembleDiagonalPA(Vector&) { SerialAssemblyOnly(); }
void ElasticityIntegrator::AddMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void ElasticityIntegrator::AddMultTransposePA(const Vector&, Vector&) const { SerialAssemblyOnly(); }

void ElasticityComponentIntegrator::AssemblePA(const FiniteElementSpace&) { SerialAssemblyOnly(); }
void ElasticityComponentIntegrator::AssembleEA(const FiniteElementSpace&, Vector&, const bool) { SerialAssemblyOnly(); }
void ElasticityComponentIntegrator::AddMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void ElasticityComponentIntegrator::AddMultTransposePA(const Vector&, Vector&) const { SerialAssemblyOnly(); }

void VectorMassIntegrator::AssemblePA(const FiniteElementSpace&) { SerialAssemblyOnly(); }
void VectorMassIntegrator::AssembleMF(const FiniteElementSpace&) { SerialAssemblyOnly(); }
void VectorMassIntegrator::AssembleDiagonalPA(Vector&) { SerialAssemblyOnly(); }
void VectorMassIntegrator::AssembleDiagonalMF(Vector&) { SerialAssemblyOnly(); }
void VectorMassIntegrator::AddMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void VectorMassIntegrator::AddMultMF(const Vector&, Vector&) const { SerialAssemblyOnly(); }

void VectorDiffusionIntegrator::AssemblePA(const FiniteElementSpace&) { SerialAssemblyOnly(); }
void VectorDiffusionIntegrator::AssembleMF(const FiniteElementSpace&) { SerialAssemblyOnly(); }
void VectorDiffusionIntegrator::AssembleDiagonalPA(Vector&) { SerialAssemblyOnly(); }
void VectorDiffusionIntegrator::AssembleDiagonalMF(Vector&) { SerialAssemblyOnly(); }
void VectorDiffusionIntegrator::AddMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void VectorDiffusionIntegrator::AddMultMF(const Vector&, Vector&) const { SerialAssemblyOnly(); }

void VectorFEMassIntegrator::AssemblePA(const FiniteElementSpace&) { SerialAssemblyOnly(); }
void VectorFEMassIntegrator::AssemblePA(const FiniteElementSpace&, const FiniteElementSpace&) { SerialAssemblyOnly(); }
void VectorFEMassIntegrator::AssembleEA(const FiniteElementSpace&, Vector&, const bool) { SerialAssemblyOnly(); }
void VectorFEMassIntegrator::AssembleDiagonalPA(Vector&) { SerialAssemblyOnly(); }
void VectorFEMassIntegrator::AddMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void VectorFEMassIntegrator::AddAbsMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void VectorFEMassIntegrator::AddMultTransposePA(const Vector&, Vector&) const { SerialAssemblyOnly(); }

void DivDivIntegrator::AssemblePA(const FiniteElementSpace&) { SerialAssemblyOnly(); }
void DivDivIntegrator::AssembleEA(const FiniteElementSpace&, Vector&, const bool) { SerialAssemblyOnly(); }
void DivDivIntegrator::AssembleDiagonalPA(Vector&) { SerialAssemblyOnly(); }
void DivDivIntegrator::AddMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }

void VectorDivergenceIntegrator::AssemblePA(const FiniteElementSpace&, const FiniteElementSpace&) { SerialAssemblyOnly(); }
void VectorDivergenceIntegrator::AddMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void VectorDivergenceIntegrator::AddMultTransposePA(const Vector&, Vector&) const { SerialAssemblyOnly(); }

void VectorFEDivergenceIntegrator::AddMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void VectorFEDivergenceIntegrator::AddMultTransposePA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void VectorFEDivergenceIntegrator::AssembleDiagonalPA_ADAt(const Vector&, Vector&) { SerialAssemblyOnly(); }

void GradientIntegrator::AssemblePA(const FiniteElementSpace&, const FiniteElementSpace&) { SerialAssemblyOnly(); }
void GradientIntegrator::AddMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void GradientIntegrator::AddMultTransposePA(const Vector&, Vector&) const { SerialAssemblyOnly(); }

void MixedScalarCurlIntegrator::AssemblePA(const FiniteElementSpace&, const FiniteElementSpace&) { SerialAssemblyOnly(); }
void MixedScalarCurlIntegrator::AddMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void MixedScalarCurlIntegrator::AddMultTransposePA(const Vector&, Vector&) const { SerialAssemblyOnly(); }

void MixedVectorGradientIntegrator::AssemblePA(const FiniteElementSpace&, const FiniteElementSpace&) { SerialAssemblyOnly(); }
void MixedVectorGradientIntegrator::AddMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void MixedVectorGradientIntegrator::AddMultTransposePA(const Vector&, Vector&) const { SerialAssemblyOnly(); }

void MixedVectorCurlIntegrator::AssemblePA(const FiniteElementSpace&, const FiniteElementSpace&) { SerialAssemblyOnly(); }
void MixedVectorCurlIntegrator::AddMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void MixedVectorCurlIntegrator::AddMultTransposePA(const Vector&, Vector&) const { SerialAssemblyOnly(); }

void MixedVectorWeakCurlIntegrator::AssemblePA(const FiniteElementSpace&, const FiniteElementSpace&) { SerialAssemblyOnly(); }
void MixedVectorWeakCurlIntegrator::AddMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void MixedVectorWeakCurlIntegrator::AddMultTransposePA(const Vector&, Vector&) const { SerialAssemblyOnly(); }

DGTraceIntegrator::Kernels::Kernels() { }
void DGTraceIntegrator::AssemblePAInteriorFaces(const FiniteElementSpace&) { SerialAssemblyOnly(); }
void DGTraceIntegrator::AssemblePABoundaryFaces(const FiniteElementSpace&) { SerialAssemblyOnly(); }
void DGTraceIntegrator::AssembleEAInteriorFaces(const FiniteElementSpace&, Vector&, Vector&, const bool) { SerialAssemblyOnly(); }
void DGTraceIntegrator::AssembleEABoundaryFaces(const FiniteElementSpace&, Vector&, const bool) { SerialAssemblyOnly(); }
void DGTraceIntegrator::AddMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void DGTraceIntegrator::AddMultTransposePA(const Vector&, Vector&) const { SerialAssemblyOnly(); }

DGDiffusionIntegrator::Kernels::Kernels() { }
void DGDiffusionIntegrator::AssemblePAInteriorFaces(const FiniteElementSpace&) { SerialAssemblyOnly(); }
void DGDiffusionIntegrator::AssemblePABoundaryFaces(const FiniteElementSpace&) { SerialAssemblyOnly(); }
void DGDiffusionIntegrator::AddMultPAFaceNormalDerivatives(const Vector&, const Vector&, Vector&, Vector&) const { SerialAssemblyOnly(); }

void TransposeIntegrator::AssembleEA(const FiniteElementSpace&, Vector&, const bool) { SerialAssemblyOnly(); }
void TransposeIntegrator::AssembleEAInteriorFaces(const FiniteElementSpace&, Vector&, Vector&, const bool) { SerialAssemblyOnly(); }
void TransposeIntegrator::AssembleEABoundaryFaces(const FiniteElementSpace&, Vector&, const bool) { SerialAssemblyOnly(); }

void NormalTraceJumpIntegrator::AssembleEAInteriorFaces(const FiniteElementSpace&, const FiniteElementSpace&, Vector&, const bool) { SerialAssemblyOnly(); }

void GradientInterpolator::AssemblePA(const FiniteElementSpace&, const FiniteElementSpace&) { SerialAssemblyOnly(); }
void GradientInterpolator::AddMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void GradientInterpolator::AddMultTransposePA(const Vector&, Vector&) const { SerialAssemblyOnly(); }

void IdentityInterpolator::AssemblePA(const FiniteElementSpace&, const FiniteElementSpace&) { SerialAssemblyOnly(); }
void IdentityInterpolator::AddMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void IdentityInterpolator::AddMultTransposePA(const Vector&, Vector&) const { SerialAssemblyOnly(); }

void VectorConvectionNLFIntegrator::AssemblePA(const FiniteElementSpace&) { SerialAssemblyOnly(); }
void VectorConvectionNLFIntegrator::AssembleMF(const FiniteElementSpace&) { SerialAssemblyOnly(); }
void VectorConvectionNLFIntegrator::AddMultPA(const Vector&, Vector&) const { SerialAssemblyOnly(); }
void VectorConvectionNLFIntegrator::AddMultMF(const Vector&, Vector&) const { SerialAssemblyOnly(); }

} // namespace mfem

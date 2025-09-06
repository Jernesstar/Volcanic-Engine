#include "FileDialog.h"

#include "UIRenderer.h"

namespace Magma::UI {

void FileDialog::Draw() {
	UIRenderer::DrawFileDialog(*this);
}

}
for xml in $(find VolcanicWindow/.vendor/glfw/deps -name "*.xml"); do
  name=$(basename "$xml" .xml)
  echo "Generating $name..."
  wayland-scanner client-header "$xml" "VolcanicWindow/.vendor/glfw/deps/wayland/${name}-client-protocol.h"
  wayland-scanner private-code "$xml" "VolcanicWindow/.vendor/glfw/deps/wayland/${name}-client-protocol-code.h"
done
ecore_imf_evas_header_src = [
  'Ecore_IMF_Evas.h'
]

ecore_imf_evas_src = files([
  'ecore_imf_evas.c'
])

ecore_imf_evas_deps = [ecore_imf, evas]
ecore_imf_evas_pub_deps = [eina, eo, efl]
ecore_imf_evas_ext_deps = [m]

ecore_imf_evas_lib = library('ecore_imf_evas',
    ecore_imf_evas_src,
    dependencies: ecore_imf_evas_pub_deps + ecore_imf_evas_deps + ecore_imf_evas_ext_deps,
    include_directories : config_dir + [include_directories('.')],
    install: true,
    c_args : package_c_args,
    version : meson.project_version()
)

ecore_imf_evas = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ecore_imf_evas_lib,
  sources : pub_eo_file_target + priv_eo_file_target,
  dependencies: ecore_imf_evas_pub_deps,
)

install_headers(ecore_imf_evas_header_src,
  install_dir : dir_package_include,
)

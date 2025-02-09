project('task-tui', 'cpp',
  version : '0.1',
  default_options : ['warning_level=everything', 'cpp_std=c++23'])
if get_option('buildtype') == 'debug'
  add_project_arguments('-ggdb',  language : 'cpp')
endif

# I think due to the upstream packaging, the cmake wrap exposes ftxui as three seperate dependencies
# Is there any other way to "bundle" the dependencies as one, without writing a meson.build for ftxui?
ftxui_dom_dep = dependency('ftxui-dom', version : '>=5.0.0')
ftxui_screen_dep = dependency('ftxui-screen', version : '>=5.0.0')
ftxui_component_dep = dependency('ftxui-component', version : '>=5.0.0')
ftxui_dep = [ftxui_dom_dep, ftxui_screen_dep, ftxui_component_dep]
#cmake = import('cmake')
#ftxui_subproj = cmake.subproject('FTXUI')
#ftxui_dom_dep = ftxui_subproj.dependency('dom')
#ftxui_screen_dep = ftxui_subproj.dependency('screen')
#ftxui_component_dep = ftxui_subproj.dependency('component')

boost_dep = dependency('boost')

executable('task-tui',
           'main.cpp',
           'components.cpp',
           dependencies : [ftxui_dep, boost_dep],
           install : true)

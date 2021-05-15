def __bootstrap__():
    global __bootstrap__, __loader__, __file__
    import sys, pkg_resources
    from importlib.machinery import ExtensionFileLoader
    __file__ = pkg_resources.resource_filename(__name__, '_sodium.cp37-win_amd64.pyd')
    __loader__ = None; del __bootstrap__, __loader__
    ExtensionFileLoader(__name__,__file__).load_module()
__bootstrap__()

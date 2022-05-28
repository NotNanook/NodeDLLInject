const dllInjector = require('./build/Release/nodedllinject.node');
result = dllInjector.execute("C:/Users/gabrys/AppData/Local/RAoT/raot.exe");
dllInjector.inject("C:/Users/gabrys/Documents/ClientCoding/Ascendit-Raot/x64/Debug/Ascendit-Raot.dll", "raot.exe");
module.exports = dllInjector;
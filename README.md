# NodeDLLInject

Made in N-API with the help of [this](https://morioh.com/p/e1a6f79af449) <br>
Fully context-aware so you can use it in Electron

## Usage (Simply follow the tutorial linked above)
To built: `npm run build` and use `require()` to access it in your Electron/Node project
DLL is inject with `LoadLibrary`

### Syntax
`execute(<path to exe>)` - Starts an application <br>
`inject(<path to dll>, <process name>)` - Inject into application

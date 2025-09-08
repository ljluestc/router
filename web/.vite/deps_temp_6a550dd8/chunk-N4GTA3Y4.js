import {
  DefaultPropsProvider_default,
  require_jsx_runtime,
  useDefaultProps
} from "./chunk-2K6NNDDG.js";
import {
  require_prop_types
} from "./chunk-6OH7SSYC.js";
import {
  _extends,
  init_extends
} from "./chunk-MPG3HUSC.js";
import {
  __toESM,
  require_react
} from "./chunk-KRQVU3GS.js";

// node_modules/@mui/material/DefaultPropsProvider/DefaultPropsProvider.js
init_extends();
var React = __toESM(require_react());
var import_prop_types = __toESM(require_prop_types());
var import_jsx_runtime = __toESM(require_jsx_runtime());
function DefaultPropsProvider(props) {
  return (0, import_jsx_runtime.jsx)(DefaultPropsProvider_default, _extends({}, props));
}
true ? DefaultPropsProvider.propTypes = {
  // ┌────────────────────────────── Warning ──────────────────────────────┐
  // │ These PropTypes are generated from the TypeScript type definitions. │
  // │ To update them, edit the TypeScript types and run `pnpm proptypes`. │
  // └─────────────────────────────────────────────────────────────────────┘
  /**
   * @ignore
   */
  children: import_prop_types.default.node,
  /**
   * @ignore
   */
  value: import_prop_types.default.object.isRequired
} : void 0;
function useDefaultProps2(params) {
  return useDefaultProps(params);
}

export {
  useDefaultProps2 as useDefaultProps
};
//# sourceMappingURL=chunk-N4GTA3Y4.js.map

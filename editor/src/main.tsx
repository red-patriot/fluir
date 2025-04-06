import { StrictMode } from "react";
import { createRoot } from "react-dom/client";
import "./index.css";
import DisplayFluir from "./DisplayFluir";

createRoot(document.getElementById("root")!).render(
  <StrictMode>
    <DisplayFluir />
  </StrictMode>
);

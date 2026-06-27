import { describe, it, expect, afterEach, vi } from "vitest";
import { createElement, ElementType } from "react";
import { ReactFlowProvider } from "@xyflow/react";
import { screen, fireEvent } from "@testing-library/react";
import userEvent from "@testing-library/user-event";
import "@testing-library/jest-dom";
import axios from "axios";
import { SERVER_API } from "@/api";
import { renderWithStore } from "@/utility/testStore";
import {
  BinaryOperatorNode,
  UnaryOperatorNode,
} from "@/components/flow_diagram/elements/OperatorNode";

vi.mock("axios");

const renderNode = (component: ElementType, fullID: string) =>
  renderWithStore(
    <ReactFlowProvider>
      {createElement(component, {
        // eslint-disable-next-line @typescript-eslint/no-explicit-any
        data: { operator: { op: "+" }, fullID },
        // eslint-disable-next-line @typescript-eslint/no-explicit-any
      } as any)}
    </ReactFlowProvider>,
    {
      preloadedState: {
        // eslint-disable-next-line @typescript-eslint/no-explicit-any
        program: { path: "foo.fl" } as any,
      },
    },
  );

describe("OperatorNode dynamic choices", () => {
  afterEach(vi.resetAllMocks);

  it("fetches binary operators (arity 2) when the popover opens", async () => {
    vi.mocked(axios.post).mockResolvedValue({ data: ["+", "&&", "=="] });

    renderNode(BinaryOperatorNode, "0:1");

    // No request until the user starts editing.
    expect(axios.post).not.toHaveBeenCalled();

    await userEvent.click(screen.getByLabelText("0:1-value-display"));

    expect(await screen.findByText("&&")).toBeVisible();
    expect(axios.post).toHaveBeenCalledWith(SERVER_API.operators, {
      operator_id: [0, 1],
      arity: 2,
      path: "foo.fl",
    });
  });

  it("fetches unary operators (arity 1) when the popover opens", async () => {
    vi.mocked(axios.post).mockResolvedValue({ data: ["+", "!", "--"] });

    renderNode(UnaryOperatorNode, "2");

    await userEvent.click(screen.getByLabelText("2-value-display"));

    expect(await screen.findByText("!")).toBeVisible();
    expect(axios.post).toHaveBeenCalledWith(SERVER_API.operators, {
      operator_id: [2],
      arity: 1,
      path: "foo.fl",
    });
  });

  it("updates the operator when a fetched choice is selected", async () => {
    vi.mocked(axios.post).mockResolvedValue({ data: ["+", "&&"] });

    renderNode(BinaryOperatorNode, "0:1");

    await userEvent.click(screen.getByLabelText("0:1-value-display"));
    const choice = await screen.findByLabelText("choice-&&");
    fireEvent.click(choice);

    // popover closes after selection
    expect(screen.queryByLabelText("choice-&&")).not.toBeInTheDocument();
  });
});

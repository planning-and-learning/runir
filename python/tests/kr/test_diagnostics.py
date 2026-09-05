import gc

import pytest

from pyrunir.kr import DuplicateDefinitionError, ParseError, SemanticError
from pyrunir.kr.dl.base.semantics import ConstructorRepositoryFactory
from pyrunir.kr.ps.base import RepositoryFactory
from pyrunir.kr.ps.base.dl import parse_sketch
from pytyr.formalism.planning import PlanningDomain
from pyyggdrasil.diagnostics import Diagnostic, format_diagnostic


@pytest.mark.parametrize("suffix", ["", ")"])
def test_parser_rejects_missing_and_extra_closes_with_owned_context(
    gripper_planning_domain: PlanningDomain, suffix: str
) -> None:
    source = "; UTF-8 é, ignored )\n(:sketch (:features) (:rules)" + suffix
    if suffix:
        source += ")"
    repository = RepositoryFactory().create(ConstructorRepositoryFactory().create(gripper_planning_domain))
    with pytest.raises(ParseError) as raised:
        parse_sketch(source, gripper_planning_domain, repository)

    error = raised.value
    assert isinstance(error, SemanticError)
    diagnostic = error.diagnostic
    assert isinstance(diagnostic, Diagnostic)
    assert "while parsing sketch" in diagnostic.message
    location = diagnostic.location
    assert location is not None
    expected = len(source.encode("utf-8")) - bool(suffix)
    assert (location.begin, location.end, location.line) == (expected, expected, 2)
    assert location.source.text == source
    assert len(diagnostic.notes) == 1
    note = diagnostic.notes[0]
    assert note.message == "sketch starts here"
    assert note.location is not None
    assert note.location.begin == len(source[:source.index("(:sketch")].encode("utf-8"))
    assert note.location.source is location.source
    assert str(error) == format_diagnostic(diagnostic)
    with pytest.raises(AttributeError):
        setattr(error, "diagnostic", None)

    del error, raised, source, repository
    gc.collect()
    assert "(:sketch" in location.source.text
    assert "sketch starts here" in format_diagnostic(diagnostic)


def test_parser_preserves_semicolon_comments(gripper_planning_domain: PlanningDomain) -> None:
    source = "(:sketch\n; )\n (:features) (:rules)) ; ("
    repository = RepositoryFactory().create(ConstructorRepositoryFactory().create(gripper_planning_domain))
    parse_sketch(source, gripper_planning_domain, repository)


@pytest.mark.parametrize(
    ("expression", "owner", "unexpected"),
    [
        ("(c_and c_at_least 1 r_universal c_top)", "c_and", "c_at_least"),
        ("(c_and (c_at_least 1 r_universal) (c_top))", "c_at_least", "r_universal"),
        ("(c_not (c_andrew))", "c_not", "(c_andrew)"),
    ],
)
def test_constructor_error_belongs_to_deepest_recognized_parent(
    gripper_planning_domain: PlanningDomain, expression: str, owner: str, unexpected: str
) -> None:
    source = (
        "(:sketch (:features (:boolean (:symbol b) (:expression "
        f"(b_nonempty {expression})"
        "))) (:rules))"
    )
    repository = RepositoryFactory().create(ConstructorRepositoryFactory().create(gripper_planning_domain))
    with pytest.raises(ParseError) as raised:
        parse_sketch(source, gripper_planning_domain, repository)
    diagnostic = raised.value.diagnostic
    assert diagnostic.message.endswith(f"while parsing {owner}")
    assert diagnostic.location is not None
    assert diagnostic.location.begin == source.index(unexpected)
    assert len(diagnostic.notes) == 1
    note = diagnostic.notes[0]
    assert note.message == f"{owner} starts here"
    assert note.location is not None
    assert note.location.begin == source.index(f"({owner}")


@pytest.mark.parametrize(
    ("expression", "owner", "expected", "unexpected"),
    [
        ("(c_atomic_state room)", "c_atomic_state", "quoted predicate name", "room"),
        ("(c_nominal rooma)", "c_nominal", "quoted object name", "rooma"),
        ('(c_atomic_state "room)', "quoted predicate name", "'\"'", ")"),
        ('(c_nominal "rooma)', "quoted object name", "'\"'", ")"),
        ('(c_atomic_state " room")', "quoted predicate name", "predicate name", " room"),
        ('(c_atomic_state "room ")', "quoted predicate name", "'\"'", ' "'),
        ('(c_nominal "; comment\nrooma")', "quoted object name", "object name", ";"),
        ('(c_nominal "rooma; comment\n")', "quoted object name", "'\"'", ";"),
        ('(c_atomic_goal "room" maybe)', "c_atomic_goal", "true or false", "maybe"),
        ("(c_at_least invalid (r_universal))", "c_at_least", "unsigned integer", "invalid"),
    ],
)
def test_constructor_expectations_use_complete_readable_messages(
    gripper_planning_domain: PlanningDomain,
    expression: str,
    owner: str,
    expected: str,
    unexpected: str,
) -> None:
    source = (
        "(:sketch (:features (:boolean (:symbol b) (:expression "
        f"(b_nonempty {expression})"
        "))) (:rules))"
    )
    repository = RepositoryFactory().create(ConstructorRepositoryFactory().create(gripper_planning_domain))
    with pytest.raises(ParseError) as raised:
        parse_sketch(source, gripper_planning_domain, repository)
    diagnostic = raised.value.diagnostic
    assert diagnostic.message == f"Expected {expected} while parsing {owner}"
    assert diagnostic.location is not None
    start = source.index('"') if owner.startswith("quoted") else source.index(f"({owner}")
    assert diagnostic.location.begin == source.index(unexpected, start)
    assert diagnostic.location.end == diagnostic.location.begin
    assert len(diagnostic.notes) == 1
    note = diagnostic.notes[0]
    assert note.message == f"{owner} starts here"
    assert note.location is not None
    assert note.location.begin == start
    assert str(raised.value) == format_diagnostic(diagnostic)


@pytest.mark.parametrize(
    ("source", "message"),
    [
        ("(:sketch (:unknown))", "Expected :features while parsing sketch"),
        ("(:sketch (:features) (:unknown))", "Expected :rules while parsing sketch"),
        (
            "(:sketch (:features (:boolean (:unknown))))",
            "Expected :symbol while parsing boolean feature",
        ),
    ],
)
def test_missing_sections_have_complete_readable_messages(
    gripper_planning_domain: PlanningDomain, source: str, message: str
) -> None:
    repository = RepositoryFactory().create(ConstructorRepositoryFactory().create(gripper_planning_domain))
    with pytest.raises(ParseError) as raised:
        parse_sketch(source, gripper_planning_domain, repository)
    assert raised.value.diagnostic.message == message
    assert raised.value.diagnostic.location is not None
    assert raised.value.diagnostic.location.begin == source.index("(:unknown")


def test_semantic_error_retains_exact_identifier_span(gripper_planning_domain: PlanningDomain) -> None:
    source = (
        "(:sketch (:features) (:rules "
        "(:rule (:symbol same) (:expression (:conditions) (:effects))) "
        "(:rule (:symbol same) (:expression (:conditions) (:effects)))))"
    )
    repository = RepositoryFactory().create(ConstructorRepositoryFactory().create(gripper_planning_domain))
    with pytest.raises(DuplicateDefinitionError, match="Duplicate rule definition: same") as raised:
        parse_sketch(source, gripper_planning_domain, repository)
    diagnostic = raised.value.diagnostic
    location = diagnostic.location
    assert location is not None
    begin = source.rindex("same")
    assert (location.begin, location.end) == (begin, begin + len("same"))
    assert location.source.text[location.begin:location.end] == "same"
    assert diagnostic.notes == []


def test_manually_constructed_exceptions_have_locationless_diagnostics() -> None:
    error = ParseError("invalid input")
    assert isinstance(error, SemanticError)
    assert error.diagnostic.message == "invalid input"
    assert error.diagnostic.location is None
    assert error.diagnostic.notes == []

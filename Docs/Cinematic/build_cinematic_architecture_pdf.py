# -*- coding: utf-8 -*-
from __future__ import annotations

from pathlib import Path
from xml.sax.saxutils import escape

from reportlab.lib import colors
from reportlab.lib.enums import TA_LEFT
from reportlab.lib.pagesizes import LETTER
from reportlab.lib.styles import ParagraphStyle, getSampleStyleSheet
from reportlab.lib.units import inch
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.platypus import (
    ListFlowable,
    ListItem,
    PageBreak,
    Paragraph,
    Preformatted,
    SimpleDocTemplate,
    Spacer,
    Table,
    TableStyle,
)


OUT_DIR = Path(__file__).resolve().parent
PDF_PATH = OUT_DIR / "Project_MJS_Cinematic_Architecture.pdf"
FONT_PATH = Path(r"C:\Windows\Fonts\malgun.ttf")
FONT_BOLD_PATH = Path(r"C:\Windows\Fonts\malgunbd.ttf")

PAGE_WIDTH, PAGE_HEIGHT = LETTER
CONTENT_WIDTH = PAGE_WIDTH - (2 * inch)


def register_fonts() -> tuple[str, str]:
    regular = "MalgunGothic"
    bold = "MalgunGothicBold"
    pdfmetrics.registerFont(TTFont(regular, str(FONT_PATH)))
    pdfmetrics.registerFont(TTFont(bold, str(FONT_BOLD_PATH)))
    return regular, bold


FONT, FONT_BOLD = register_fonts()


def make_styles() -> dict[str, ParagraphStyle]:
    base = getSampleStyleSheet()
    return {
        "title": ParagraphStyle(
            "KTitle",
            parent=base["Title"],
            fontName=FONT_BOLD,
            fontSize=22,
            leading=28,
            textColor=colors.HexColor("#111827"),
            spaceAfter=8,
            alignment=TA_LEFT,
            wordWrap="CJK",
        ),
        "subtitle": ParagraphStyle(
            "KSubtitle",
            parent=base["Normal"],
            fontName=FONT,
            fontSize=10.5,
            leading=15,
            textColor=colors.HexColor("#4B5563"),
            spaceAfter=14,
            wordWrap="CJK",
        ),
        "h1": ParagraphStyle(
            "KH1",
            parent=base["Heading1"],
            fontName=FONT_BOLD,
            fontSize=15.5,
            leading=20,
            textColor=colors.HexColor("#1F5E91"),
            spaceBefore=13,
            spaceAfter=7,
            keepWithNext=True,
            wordWrap="CJK",
        ),
        "h2": ParagraphStyle(
            "KH2",
            parent=base["Heading2"],
            fontName=FONT_BOLD,
            fontSize=12.5,
            leading=17,
            textColor=colors.HexColor("#17496F"),
            spaceBefore=9,
            spaceAfter=5,
            keepWithNext=True,
            wordWrap="CJK",
        ),
        "body": ParagraphStyle(
            "KBody",
            parent=base["BodyText"],
            fontName=FONT,
            fontSize=9.8,
            leading=14.2,
            textColor=colors.HexColor("#18202B"),
            spaceAfter=5.5,
            wordWrap="CJK",
        ),
        "small": ParagraphStyle(
            "KSmall",
            parent=base["BodyText"],
            fontName=FONT,
            fontSize=8.4,
            leading=11.5,
            textColor=colors.HexColor("#4B5563"),
            spaceAfter=4,
            wordWrap="CJK",
        ),
        "table": ParagraphStyle(
            "KTable",
            parent=base["BodyText"],
            fontName=FONT,
            fontSize=8.2,
            leading=10.8,
            textColor=colors.HexColor("#18202B"),
            wordWrap="CJK",
        ),
        "table_header": ParagraphStyle(
            "KTableHeader",
            parent=base["BodyText"],
            fontName=FONT_BOLD,
            fontSize=8.3,
            leading=10.8,
            textColor=colors.HexColor("#17496F"),
            wordWrap="CJK",
        ),
        "callout_title": ParagraphStyle(
            "KCalloutTitle",
            parent=base["BodyText"],
            fontName=FONT_BOLD,
            fontSize=10,
            leading=13,
            textColor=colors.HexColor("#17496F"),
            spaceAfter=3,
            wordWrap="CJK",
        ),
        "code": ParagraphStyle(
            "KCode",
            parent=base["Code"],
            fontName=FONT,
            fontSize=8.2,
            leading=10.5,
            textColor=colors.HexColor("#26313F"),
        ),
    }


S = make_styles()


def p(text: str, style: str = "body") -> Paragraph:
    safe_text = escape(text).replace("\n", "<br/>")
    return Paragraph(safe_text, S[style])


def bullet_list(items: list[str]) -> ListFlowable:
    return ListFlowable(
        [ListItem(p(item), leftIndent=11) for item in items],
        bulletType="bullet",
        start="circle",
        leftIndent=17,
        bulletFontName=FONT,
        bulletFontSize=7.5,
    )


def numbered_list(items: list[str]) -> ListFlowable:
    return ListFlowable(
        [ListItem(p(item), leftIndent=15) for item in items],
        bulletType="1",
        leftIndent=22,
        bulletFontName=FONT,
        bulletFontSize=8.5,
    )


def callout(title: str, body: str, fill: str = "#E8EEF5") -> Table:
    table = Table(
        [[p(title, "callout_title")], [p(body, "body")]],
        colWidths=[CONTENT_WIDTH],
        splitByRow=True,
    )
    table.setStyle(
        TableStyle(
            [
                ("BACKGROUND", (0, 0), (-1, -1), colors.HexColor(fill)),
                ("BOX", (0, 0), (-1, -1), 0.6, colors.HexColor("#B8C7D9")),
                ("LEFTPADDING", (0, 0), (-1, -1), 10),
                ("RIGHTPADDING", (0, 0), (-1, -1), 10),
                ("TOPPADDING", (0, 0), (-1, -1), 7),
                ("BOTTOMPADDING", (0, 0), (-1, -1), 7),
            ]
        )
    )
    return table


def code_block(text: str) -> Table:
    block = Preformatted(text, S["code"], maxLineLength=108)
    table = Table([[block]], colWidths=[CONTENT_WIDTH], splitByRow=True)
    table.setStyle(
        TableStyle(
            [
                ("BACKGROUND", (0, 0), (-1, -1), colors.HexColor("#F7F9FC")),
                ("BOX", (0, 0), (-1, -1), 0.4, colors.HexColor("#D9E0EA")),
                ("LEFTPADDING", (0, 0), (-1, -1), 9),
                ("RIGHTPADDING", (0, 0), (-1, -1), 9),
                ("TOPPADDING", (0, 0), (-1, -1), 7),
                ("BOTTOMPADDING", (0, 0), (-1, -1), 7),
            ]
        )
    )
    return table


def matrix(headers: list[str], rows: list[list[str]], widths: list[float]) -> Table:
    data = [[p(header, "table_header") for header in headers]]
    data.extend([[p(cell, "table") for cell in row] for row in rows])
    table = Table(data, colWidths=[width * inch for width in widths], repeatRows=1, splitByRow=True)
    table.setStyle(
        TableStyle(
            [
                ("BACKGROUND", (0, 0), (-1, 0), colors.HexColor("#E8EEF5")),
                ("GRID", (0, 0), (-1, -1), 0.35, colors.HexColor("#B8C7D9")),
                ("VALIGN", (0, 0), (-1, -1), "TOP"),
                ("LEFTPADDING", (0, 0), (-1, -1), 6),
                ("RIGHTPADDING", (0, 0), (-1, -1), 6),
                ("TOPPADDING", (0, 0), (-1, -1), 5),
                ("BOTTOMPADDING", (0, 0), (-1, -1), 5),
            ]
        )
    )
    return table


def kv_table(rows: list[tuple[str, str]]) -> Table:
    return matrix(["항목", "설명"], [[label, value] for label, value in rows], [1.7, 4.8])


def on_page(canvas, doc) -> None:
    canvas.saveState()
    canvas.setFont(FONT, 8)
    canvas.setFillColor(colors.HexColor("#6B7280"))
    canvas.drawString(inch, PAGE_HEIGHT - 0.55 * inch, "Project_MJS Cinematic Architecture")
    canvas.drawRightString(PAGE_WIDTH - inch, 0.5 * inch, f"Page {doc.page}")
    canvas.restoreState()


def build_story():
    story = []

    story.append(p("Project_MJS", "small"))
    story.append(p("시네마틱 시스템 구조 설명서", "title"))
    story.append(
        p(
            "스킬 시퀀스, 궁극기 컷신, 일반 레벨 컷신을 하나의 Director 기반 재생 구조로 묶고, "
            "플레이어 월드 위치/상대 위치/플레이어 회전/카메라 회전을 기준으로 동적 시퀀스를 재생하기 위한 구현 분석 문서입니다.",
            "subtitle",
        )
    )
    story.append(p("인코딩: UTF-8 / 출력: PDF / 폰트: Malgun Gothic / 기준 코드: Project_MJS Cinematic 폴더", "small"))
    story.append(Spacer(1, 8))
    story.append(
        callout(
            "핵심 결론",
            "현재 구조는 전투 로직과 시네마틱 연출을 분리하는 방향이 맞습니다. 이번 업데이트로 Level Sequence를 에셋에 저장된 월드 위치에 고정하지 않고, "
            "런타임에 플레이어, 타겟, 트리거, 카메라 방향을 기준으로 재배치할 수 있게 되었습니다. UE의 UDefaultLevelSequenceInstanceData.TransformOrigin을 사용하므로 "
            "시퀀서의 절대 Transform Section도 동적 원점 기준으로 평가됩니다.",
        )
    )

    story.append(p("1. 전체 구조", "h1"))
    story.append(
        code_block(
            "SkillComponent / CinematicActionComponent / CinematicTriggerActor\n"
            "        |\n"
            "        v\n"
            "FCinematicPlaybackRequest\n"
            "  - Sequence\n"
            "  - Instigator / Subject / Participants\n"
            "  - AnchorMode / RotationSource / RelativeTransform\n"
            "        |\n"
            "        v\n"
            "UCinematicDirectorSubsystem\n"
            "  - LevelSequencePlayer 생성\n"
            "  - LevelSequenceActor 동적 Transform Origin 적용\n"
            "  - ICinematicParticipant 시작/종료 알림\n"
            "        |\n"
            "        v\n"
            "UCinematicParticipantComponent\n"
            "  - 입력 잠금, 이동 정지, Tick 정지, 애니메이션 정지 옵션 처리"
        )
    )
    story.append(
        p(
            "시네마틱 재생 요청자는 구체적인 입력 잠금 방식이나 AI 정지 방식을 몰라도 됩니다. 요청자는 FCinematicPlaybackRequest를 만들고 Director에 넘깁니다. "
            "Director는 시퀀스를 재생하고, 실제로 어떤 액터가 멈출지는 ICinematicParticipant 구현체가 독립적으로 처리합니다."
        )
    )

    story.append(p("2. 이번 업데이트 요약", "h1"))
    story.append(
        bullet_list(
            [
                "FCinematicPlaybackRequest에 AnchorMode, RotationSource, AnchorActor, AnchorSocketName, TargetSocketName, RelativeTransform, ExplicitWorldTransform, ExplicitRotation, bUseYawOnly를 추가했습니다.",
                "FCinematicPlaybackContext에 AnchorWorldTransform과 bAppliedDynamicTransform을 추가해 참가자/디버그/VFX가 동일한 기준점을 읽을 수 있게 했습니다.",
                "UCinematicDirectorSubsystem이 LevelSequenceActor 생성 직후 동적 원점을 계산하고, UDefaultLevelSequenceInstanceData.TransformOrigin에 적용하도록 만들었습니다.",
                "UCinematicActionComponent에 PlayCinematicRequest와 PlayAnchoredCinematic을 추가해 스킬 쪽에서 구조체 요청 또는 앵커 기반 편의 호출을 사용할 수 있게 했습니다.",
                "ACinematicTriggerActor에도 동적 Transform 옵션을 노출해 일반 컷신을 월드 고정, 트리거 기준, 플레이어 기준으로 선택할 수 있게 했습니다.",
                "Build.cs에 MovieSceneTracks 의존성을 추가해 Transform Origin 관련 엔진 헤더 경로를 안정화했습니다.",
            ]
        )
    )

    story.append(p("3. 요청 데이터 구조", "h1"))
    story.append(
        matrix(
            ["필드", "역할", "대표 사용"],
            [
                ["Sequence", "재생할 ULevelSequence 에셋입니다. 비어 있으면 요청은 실패합니다.", "스킬 컷인, 궁극기, 레벨 컷신"],
                ["InstigatorActor", "시네마틱을 발동한 액터입니다.", "스킬 사용자, 트리거에 들어온 플레이어"],
                ["SubjectActor", "연출의 중심 대상입니다.", "플레이어, 타겟, 보스, NPC"],
                ["AdditionalParticipants", "명시적으로 멈추거나 반응시킬 추가 액터 목록입니다.", "보스, 소환수, 주변 NPC"],
                ["bAffectAllParticipants", "true면 월드의 모든 CinematicParticipant를 수집합니다. false면 명시된 대상만 반응합니다.", "궁극기 전체 정지, 특정 NPC만 정지"],
                ["AnchorMode", "시퀀스의 월드 원점을 어디서 계산할지 결정합니다.", "플레이어 기준, 타겟 기준, 명시 트랜스폼"],
                ["RotationSource", "최종 시퀀스 회전을 어디서 가져올지 결정합니다.", "플레이어 전방, 카메라 방향, 명시 회전"],
                ["RelativeTransform", "계산된 앵커 기준 로컬 오프셋입니다.", "플레이어 앞 150cm에서 시작"],
                ["bUseYawOnly", "Pitch/Roll을 제거하고 Yaw만 사용합니다.", "지상 액션 스킬 대부분"],
            ],
            [1.55, 3.25, 1.7],
        )
    )

    story.append(p("4. AnchorMode 해석", "h1"))
    story.append(
        matrix(
            ["AnchorMode", "계산 방식", "권장 상황"],
            [
                ["AuthoredWorld", "Level Sequence 에셋에 저장된 위치와 회전을 그대로 사용합니다. 동적 Transform Origin을 적용하지 않습니다.", "정해진 맵 위치에서만 나오는 일반 컷신"],
                ["InstigatorActor", "InstigatorActor 또는 AnchorActor의 월드 트랜스폼을 원점으로 사용합니다.", "플레이어 위치 기준 스킬 컷신"],
                ["SubjectActor", "SubjectActor 또는 AnchorActor의 월드 트랜스폼을 원점으로 사용합니다.", "보스/타겟 중심 연출"],
                ["InstigatorToSubject", "Instigator 위치를 원점으로 잡고 Subject 방향을 바라보는 회전을 계산합니다.", "플레이어가 타겟을 향해 쓰는 처형기/연계기"],
                ["ExplicitTransform", "요청자가 넘긴 ExplicitWorldTransform을 원점으로 사용합니다.", "SkillComponent가 별도 계산한 위치/회전을 그대로 적용"],
            ],
            [1.55, 3.25, 1.7],
        )
    )

    story.append(p("5. RotationSource 해석", "h1"))
    story.append(
        matrix(
            ["RotationSource", "계산 방식", "권장 상황"],
            [
                ["AnchorTransform", "AnchorMode가 계산한 회전을 그대로 사용합니다.", "일반적인 액터 기준 컷신"],
                ["InstigatorActor", "InstigatorActor의 월드 회전을 사용합니다.", "캐릭터 정면 기준 공격"],
                ["SubjectActor", "SubjectActor의 월드 회전을 사용합니다.", "타겟 전방/후방 연출"],
                ["PlayerControlRotation", "PlayerController의 ControlRotation을 사용합니다.", "조준/입력 방향 기반 스킬"],
                ["PlayerCameraRotation", "PlayerCameraManager의 현재 카메라 회전을 사용합니다.", "카메라가 보는 방향으로 나가는 스킬"],
                ["ExplicitRotation", "요청자가 넘긴 ExplicitRotation을 사용합니다.", "SkillComponent가 카메라 yaw를 직접 계산해 의존성을 줄이는 방식"],
            ],
            [1.55, 3.25, 1.7],
        )
    )

    story.append(PageBreak())
    story.append(p("6. 동적 시퀀스 실행 흐름", "h1"))
    story.append(
        numbered_list(
            [
                "요청자가 FCinematicPlaybackRequest를 생성합니다.",
                "Director가 ULevelSequencePlayer와 런타임 ALevelSequenceActor를 생성합니다.",
                "AnchorMode가 AuthoredWorld가 아니면 Director가 앵커 액터/소켓/명시 트랜스폼을 해석합니다.",
                "InstigatorToSubject 모드라면 Instigator 위치에서 Subject 위치를 바라보는 회전을 계산합니다.",
                "RotationSource가 지정되어 있으면 앵커 회전을 플레이어 회전, 카메라 회전, 명시 회전 등으로 교체합니다.",
                "bUseYawOnly가 true면 Pitch/Roll을 제거해 지상 액션 게임에 맞는 Yaw 회전만 남깁니다.",
                "RelativeTransform을 앵커에 곱해 최종 AnchorWorldTransform을 만듭니다.",
                "LevelSequenceActor의 ActorTransform과 DefaultInstanceData.TransformOrigin에 최종 Transform을 적용합니다.",
                "참가자들에게 OnCinematicStarted를 알리고 시퀀스를 재생합니다.",
                "재생 종료 시 참가자 복구, ViewTarget 복구, 런타임 SequenceActor 정리를 수행합니다.",
            ]
        )
    )
    story.append(
        code_block(
            "FinalRotation = ResolveRotation(Request, AnchorTransform)\n"
            "AnchorTransform.SetRotation(FinalRotation)\n"
            "FinalTransform = Request.RelativeTransform * AnchorTransform\n"
            "\n"
            "LevelSequenceActor.SetActorTransform(FinalTransform)\n"
            "InstanceData.TransformOrigin = FinalTransform\n"
            "LevelSequenceActor.bOverrideInstanceData = true"
        )
    )
    story.append(
        callout(
            "왜 TransformOrigin이 필요한가",
            "LevelSequenceActor의 액터 위치만 바꾸면 일부 카메라/스폰 액터에는 충분할 수 있지만, 시퀀서 안의 절대 Transform Section을 재사용 가능한 상대 연출로 만들려면 "
            "엔진이 평가 시 참조하는 Transform Origin을 넣어야 합니다. 이번 구현은 UDefaultLevelSequenceInstanceData를 통해 그 경로를 사용합니다.",
        )
    )

    story.append(p("7. 스킬 시퀀스 사용 패턴", "h1"))
    story.append(
        matrix(
            ["목표", "추천 설정", "설명"],
            [
                ["플레이어 정면 기준 궁극기", "AnchorMode=InstigatorActor, RotationSource=InstigatorActor, bUseYawOnly=true", "캐릭터가 바라보는 방향으로 컷신 원점과 VFX가 정렬됩니다."],
                ["카메라 방향 기준 스킬", "AnchorMode=InstigatorActor, RotationSource=PlayerCameraRotation, bUseYawOnly=true", "플레이어 위치에서 시작하지만 방향은 현재 카메라 yaw를 따릅니다."],
                ["타겟을 향한 처형기", "AnchorMode=InstigatorToSubject, RotationSource=AnchorTransform, SubjectActor=Target", "플레이어 위치에서 타겟을 바라보는 방향으로 시퀀스가 놓입니다."],
                ["스킬 컴포넌트가 직접 계산", "AnchorMode=ExplicitTransform 또는 RotationSource=ExplicitRotation", "Cinematic 모듈이 플레이어 컨트롤러나 카메라 구현을 몰라도 됩니다."],
                ["플레이어 앞 오프셋", "RelativeTransform.Location=(150, 0, 0)", "계산된 앵커의 로컬 X축 앞쪽으로 150cm 이동한 위치에서 재생됩니다."],
            ],
            [1.6, 2.55, 2.35],
        )
    )
    story.append(
        code_block(
            "FCinematicPlaybackRequest Request;\n"
            "Request.Sequence = UltimateSequence;\n"
            "Request.InstigatorActor = Player;\n"
            "Request.SubjectActor = Target;\n"
            "Request.AnchorMode = ECinematicAnchorMode::InstigatorToSubject;\n"
            "Request.RotationSource = ECinematicRotationSource::AnchorTransform;\n"
            "Request.RelativeTransform = FTransform(FRotator::ZeroRotator, FVector(120.0f, 0.0f, 0.0f));\n"
            "Request.bUseYawOnly = true;\n"
            "DirectorSubsystem->PlayCinematic(Request);"
        )
    )

    story.append(p("8. 일반 컷신/트리거 사용 패턴", "h1"))
    story.append(
        bullet_list(
            [
                "맵에 고정된 컷신은 AnchorMode를 AuthoredWorld로 둡니다. 시퀀서 에셋 안의 월드 위치가 그대로 사용됩니다.",
                "트리거 액터 위치를 기준으로 재생하려면 ACinematicTriggerActor에서 bUseTriggerActorAsAnchor를 켜고 AnchorMode를 InstigatorActor 또는 SubjectActor가 아닌 앵커 기반 모드로 설정합니다. AnchorActor override가 우선 적용됩니다.",
                "트리거에 들어온 플레이어 위치 기준으로 재생하려면 bUseTriggerActorAsAnchor를 끄고 AnchorMode=InstigatorActor를 사용합니다.",
                "특정 NPC나 보스 기준 컷신은 ActivateCinematic(TargetActor)에 넘기는 대상과 SubjectActor/AnchorActor 설정을 분리해서 확장할 수 있습니다.",
            ]
        )
    )

    story.append(p("9. 의존성 분리 관점", "h1"))
    story.append(
        matrix(
            ["모듈", "알아야 하는 것", "모르면 되는 것"],
            [
                ["SkillComponent", "스킬 조건, 쿨타임, 게이지, 타겟, 원하는 앵커/회전 값", "참가자 입력 잠금 방식, AI 정지 방식, 시퀀스 플레이어 내부"],
                ["CinematicDirectorSubsystem", "요청 데이터, LevelSequencePlayer, Transform Origin, 참가자 알림", "스킬 쿨타임, 데미지 판정, 개별 캐릭터 복구 정책"],
                ["CinematicParticipantComponent", "시작/종료 컨텍스트와 자신의 옵션", "요청자가 스킬인지 트리거인지, 시퀀서 카메라 컷 구성"],
                ["CinematicTriggerActor", "트리거 조건과 재생할 Sequence", "플레이어 클래스의 내부 전투 컴포넌트 구조"],
            ],
            [1.65, 2.75, 2.1],
        )
    )
    story.append(
        callout(
            "카메라 회전 의존성을 더 줄이는 방법",
            "RotationSource=PlayerCameraRotation을 쓰면 Director가 PlayerCameraManager를 읽습니다. 더 낮은 의존성을 원하면 SkillComponent가 Project_MJS의 카메라 규칙에 맞춰 yaw를 계산한 뒤 "
            "RotationSource=ExplicitRotation으로 넘기면 됩니다. 그러면 Cinematic 모듈은 카메라 구현을 거의 몰라도 됩니다.",
        )
    )

    story.append(p("10. 이번에 반영한 개선", "h1"))
    story.append(
        matrix(
            ["개선 항목", "반영 내용", "에디터/코드 사용 지점"],
            [
                ["Binding Override", "FCinematicBindingOverride를 추가해 Sequencer의 Binding Tag를 Player, Target, Weapon 같은 런타임 액터로 교체할 수 있게 했습니다. 트리거는 오버랩 액터를 Player 태그에 자동 바인딩할 수도 있습니다.", "Request.BindingOverrides 또는 ACinematicTriggerActor의 BindingOverrides / bBindTriggeringActor"],
                ["Input Lock Token", "UCinematicInputLockSubsystem을 추가해 입력 잠금을 핸들 기반 Acquire/Release로 관리합니다. ParticipantComponent는 자신이 얻은 핸들만 해제합니다.", "UCinematicParticipantComponent의 입력 잠금 처리"],
                ["AI/Brain 정지 정책", "bPauseAILogic 옵션을 추가해 AIController의 BrainComponent를 PauseLogic/ResumeLogic으로 멈추고 복구할 수 있게 했습니다.", "Enemy/NPC에 붙은 UCinematicParticipantComponent"],
                ["Network 정책", "ECinematicNetworkPolicy를 추가해 LocalOnly, AuthorityOnly, AnyNetMode 중 어느 환경에서 재생할지 Request 단위로 결정합니다.", "Request.NetworkPolicy 또는 트리거 액터의 NetworkPolicy"],
                ["디버그 시각화", "bDrawDebugAnchor, DebugDrawDuration, DebugDrawScale을 추가해 계산된 AnchorWorldTransform을 월드에 좌표축과 구체로 확인할 수 있게 했습니다.", "Request.Debug 또는 트리거 액터의 Cinematic|Debug"],
            ],
            [1.55, 2.7, 2.25],
        )
    )

    story.append(p("11. 관련 코드 위치", "h1"))
    story.append(
        kv_table(
            [
                ["Source/Project_MJS/Public/Cinematic/CinematicTypes.h", "요청/컨텍스트 구조체, AnchorMode, RotationSource, BindingOverride, NetworkPolicy, Debug 옵션"],
                ["Source/Project_MJS/Public/Cinematic/CinematicDirectorSubsystem.h", "Director 인터페이스와 동적 Transform, BindingOverride, NetworkPolicy, Debug 헬퍼 선언"],
                ["Source/Project_MJS/Private/Cinematic/CinematicDirectorSubsystem.cpp", "LevelSequencePlayer 생성, Transform Origin 적용, BindingOverride 적용, 참가자 수집/알림/복구"],
                ["Source/Project_MJS/Public/Cinematic/CinematicInputLockSubsystem.h", "시네마틱 입력 잠금 핸들 서브시스템 선언"],
                ["Source/Project_MJS/Private/Cinematic/CinematicInputLockSubsystem.cpp", "AcquireInputLock/ReleaseInputLock 구현"],
                ["Source/Project_MJS/Public/Cinematic/CinematicActionComponent.h", "간단 호출, Request 직접 호출, 앵커 기반 편의 호출"],
                ["Source/Project_MJS/Private/Cinematic/CinematicActionComponent.cpp", "요청 기본값 보정, PlayerController 자동 해석"],
                ["Source/Project_MJS/Public/Cinematic/CinematicParticipantComponent.h", "입력/이동/Tick/AI/애니메이션 정지 옵션"],
                ["Source/Project_MJS/Private/Cinematic/CinematicParticipantComponent.cpp", "입력 잠금 핸들, AI Brain Pause/Resume, 기존 복구 흐름 구현"],
                ["Source/Project_MJS/Public/Cinematic/CinematicTriggerActor.h", "트리거 액터의 동적 Transform, BindingOverride, Network, Debug 설정 노출"],
                ["Source/Project_MJS/Private/Cinematic/CinematicTriggerActor.cpp", "트리거 요청 생성과 신규 옵션 전달"],
                ["Source/Project_MJS/Project_MJS.Build.cs", "LevelSequence, MovieScene, MovieSceneTracks, AIModule 등 시네마틱 모듈 의존성"],
            ]
        )
    )

    story.append(
        callout(
            "운영 기준",
            "기본 공격은 계속 AnimMontage와 Notify가 책임지고, 스킬/궁극기/컷신 연출만 이 Cinematic 구조를 사용합니다. "
            "시퀀서는 전투 판정의 주인이 아니라 카메라, FOV, 포스트프로세스, 연출용 VFX를 제어하는 재생 레이어로 두는 것이 현재 Project_MJS 구조에 가장 안정적입니다.",
        )
    )

    return story


def build_pdf() -> None:
    doc = SimpleDocTemplate(
        str(PDF_PATH),
        pagesize=LETTER,
        rightMargin=inch,
        leftMargin=inch,
        topMargin=0.82 * inch,
        bottomMargin=0.75 * inch,
        title="Project_MJS Cinematic Architecture",
        author="Codex",
        subject="Project_MJS cinematic system architecture",
    )
    doc.build(build_story(), onFirstPage=on_page, onLaterPages=on_page)
    print(PDF_PATH)


if __name__ == "__main__":
    build_pdf()
